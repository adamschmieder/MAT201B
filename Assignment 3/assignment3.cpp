/*
Allocore Example: Flocking

Description:
This is an example implementation of a flocking algorithm. The original flocking
algorithm [1] consists of three main interactions between flockmates ("boids"):

    1) Collision avoidance (of nearby flockmates)
    2) Velocity matching (of nearby flockmates)
    3) Flock centering (of nearby flockmates)

Here, we implement 1) and 2) only. Another change from the reference source is
the use of Gaussian functions rather than inverse-squared functions for
calculating the "nearness" of flockmates. This is done primarily to avoid
infinities, but also to give smoother motions. Lastly, we give each boid a
random walk motion which helps both dissolve and redirect the flocks.

[1] Reynolds, C. W. (1987). Flocks, herds, and schools: A distributed behavioral
    model. Computer Graphics, 21(4):25–34.

Author:
Lance Putnam, Oct. 2014
*/

#include <cmath>
#include "al/app/al_App.hpp"
#include "al/graphics/al_Shapes.hpp"
#include "al/math/al_Functions.hpp"
#include "al/app/al_GUIDomain.hpp"
#include "al/math/al_Random.hpp"

using namespace al;

// A "boid" (play on bird) is one member of a flock.
class Boid {
 public:
  // Each boid has a position and velocity.
  Vec3f pos, vel;

  // Update position based on velocity and delta time
  void update(float dt) { pos += vel * dt; }
};

struct MyApp : public App {
  static const int Nb = 32;  // Number of boids
  Boid boids[Nb];
  Mesh heads, tails;
  Mesh box;
  VAOMesh mCube;

  Vec3f center;

  Parameter pushRadius{"/pushRadius", "", 0.05, "", 0.01, 0.3};
  Parameter pushStrength{"/pushStrength", "", 2, "", 1, 10};
  Parameter matchRadius{"/matchRadius", "", 0.2, "", 0.05, 1.0};
  Parameter huntUrge{"/huntUrge", "", 0.2, "", 0.1, 1.0};
  Parameter localRadius{"/localRadius", "", 1.5, "", 0.1, 5.0};
  

  double angle{0};

  void onInit() override {
    auto GUIdomain = GUIDomain::enableGUI(defaultWindowDomain());
    auto &gui = GUIdomain->newGUI();
    gui.add(pushRadius);  // add parameter to GUI
    gui.add(pushStrength);   // add parameter to GUI
    gui.add(matchRadius);
    gui.add(huntUrge);
    gui.add(localRadius);
  }

  void onCreate() {
    addCube(mCube);
    mCube.primitive(Mesh::LINE_LOOP);
    mCube.scale(4);
    mCube.update();
    nav().pos(0, 0, 10);
    resetBoids();
  }

  // Randomize boid positions/velocities uniformly inside unit disc
  void resetBoids() {
    for (auto& b : boids) {
      b.pos = rnd::ball<Vec3f>();
      b.vel = rnd::ball<Vec3f>();
    }
  }

  void onAnimate(double dt_ms) {
    float dt = dt_ms;
    angle += 0.1;

    // Compute boid-boid interactions
    for (int i = 0; i < Nb - 1; ++i) {
      for (int j = i + 1; j < Nb; ++j) {
        // printf("checking boids %d and %d\n", i,j);
        auto ds = boids[i].pos - boids[j].pos;

        auto dist = ds.mag();
        // distance magnitude between the two boids

        // Collision avoidance
        float push = exp(-al::pow2(dist / pushRadius)) * pushStrength;
        // e^(-2^(dist/pushRadius)) * pushStrength

        auto pushVector = ds.normalized() * push;
        boids[i].pos = boids[i].pos + pushVector;
        boids[j].pos = boids[j].pos - pushVector;

        // Velocity matching
        float nearness = exp(-al::pow2(dist / matchRadius));
        Vec3f veli = boids[i].vel;
        Vec3f velj = boids[j].vel;

        // Take a weighted average of velocities according to nearness
        boids[i].vel = veli * (1 - 0.5 * nearness) + velj * (0.5 * nearness);
        boids[j].vel = velj * (1 - 0.5 * nearness) + veli * (0.5 * nearness);
      }
    }

    // Flock Centering
    for (int b = 0; b < Nb; b++) {
      Boid point =  boids[b];
      int count = 0;
      Vec3f sum(0, 0, 0);

      for (int i = 0; i < Nb; i++) {
        if (b != i) {
          if ((point.pos - boids[i].pos).mag() < localRadius) {
            sum = sum + boids[i].pos;
            count++;
          }
        }
      }
      if (count == 0)
        center = boids[b].pos;
      else
        center = sum / count;
      boids[b].pos = boids[b].pos - (center * 0.01);
      //std::cout << center << std::endl;
    }

    // Update boid independent behaviors
    for (auto& b : boids) {
      // Random "hunting" motion
      auto hunt = rnd::ball<Vec3f>();
      // Use cubed distribution to make small jumps more frequent
      hunt *= hunt.magSqr();
      b.vel += hunt * huntUrge;

      // Wrapping of boids within bounds
      if (b.pos.x > 2) b.pos.x = -2;
      if (b.pos.x < -2) b.pos.x = 2;
      if (b.pos.y > 2) b.pos.y = -2;
      if (b.pos.y < -2) b.pos.y = 2;
      if (b.pos.z > 2) b.pos.z = -2;
      if (b.pos.z < -2) b.pos.z = 2;
    }

    // Generate meshes
    heads.reset();
    heads.primitive(Mesh::POINTS);

    tails.reset();
    tails.primitive(Mesh::LINES);
    

    for (int i = 0; i < Nb; ++i) {
      boids[i].update(dt);

      heads.vertex(boids[i].pos);
      heads.color(HSV(float(i) / Nb * 0.3 + 0.3, 0.7));

      tails.vertex(boids[i].pos);
      tails.vertex(boids[i].pos - boids[i].vel.normalized(0.07));

      tails.color(heads.colors()[i]);
      tails.color(RGB(0.5));
    }
  }

  void onDraw(Graphics& g) {
    g.clear(0);
    g.depthTesting(true);
    g.pointSize(8);

    g.rotate(angle, Vec3d(0, 1, 0));
    // g.nicest();
    // g.stroke(8);
    g.meshColor();
    g.draw(heads);
    g.draw(tails);

    // g.stroke(1);
    g.color(1);
    g.draw(mCube);
  }

  bool onKeyDown(const Keyboard& k) {
    switch (k.key()) {
      case 'r':
        resetBoids();
        break;
    }
    return true;
  }
};

int main() {
  MyApp app;
  app.configureAudio(48000, 512, 2, 0);
  app.start();
}
