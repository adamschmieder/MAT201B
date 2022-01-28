// Karl Yerkes
// 2022-01-20

#include "al/app/al_App.hpp"
#include "al/app/al_GUIDomain.hpp"
#include "al/math/al_Random.hpp"

using namespace al;

#include <fstream>
#include <vector>
using namespace std;

Vec3f randomVec3f(float scale) {
  //takes in scale parameter
  return Vec3f(rnd::uniformS(), rnd::uniformS(), rnd::uniformS()) * scale;
}
string slurp(string fileName);  // forward declaration

struct AlloApp : App {
  Parameter pointSize{"/pointSize", "", 10.0, "", 0.0, 10.0};
  Parameter timeStep{"/timeStep", "", 0.1, "", 0.01, 0.6};
  Parameter gravConstant{"/gravConstant", "", 0.1, "", 0.0, 5.0};
  //

  ShaderProgram pointShader;

  //  simulation state
  Mesh mesh;  // position *is inside the mesh* mesh.vertices() are the positions
  vector<Vec3f> velocity;
  vector<Vec3f> acceleration;
  vector<float> mass;
  int particles = 100;
  

  void onInit() override {
    // set up GUI
    auto GUIdomain = GUIDomain::enableGUI(defaultWindowDomain());
    auto &gui = GUIdomain->newGUI();
    gui.add(pointSize);  // add parameter to GUI
    gui.add(timeStep);   // add parameter to GUI
    gui.add(gravConstant);
    //
  }

  void onCreate() override {

    // compile shaders
    pointShader.compile(slurp("../point-vertex.glsl"),
                        slurp("../point-fragment.glsl"),
                        slurp("../point-geometry.glsl"));

    // set initial conditions of the simulation
    //

    // c++11 "lambda" function
    auto randomColor = []() { return HSV(rnd::uniform(), 1.0f, 1.0f); };

    mesh.primitive(Mesh::POINTS);
    // does 1000 work on your system? how many can you make before you get a low
    // frame rate? do you need to use <1000?

    //amount of particles
    for (int _ = 0; _ < particles; _++) {
      mesh.vertex(randomVec3f(5));
      mesh.color(randomColor());

      // float m = rnd::uniform(3.0, 0.5);
      float m = 3 + rnd::normal() / 2;
      if (m < 0.5) m = 0.5;
      mass.push_back(m);

      // using a simplified volume/size relationship
      mesh.texCoord(pow(m, 1.0f / 3), 0);  // s, t

      // separate state arrays
      velocity.push_back(randomVec3f(0.1));
      acceleration.push_back(randomVec3f(1));
    }

    nav().pos(0, 0, 10);
  }

  float distance = 0;
  float distance2 = 0;
  bool freeze = false;
  float limit = 7.0;
  float limit2 = 0.5;
  double scale = 0.0;
  Vec3f v01;

  void onAnimate(double dt) override {
    if (freeze) return;

    // ignore the real dt and set the time step;
    dt = timeStep;
    
    // Calculate forces

    // F = G/(r^2)
    // F = ma, m =1, a = F
    // a = G/(r^2)

    for (int i = 0; i < particles; i++) {
      for (int j = i + 1; j < particles; j++){
        v01 = mesh.vertices()[j] - mesh.vertices()[i];
        distance = (mesh.vertices()[j] - mesh.vertices()[i]).mag();
        scale = (gravConstant/(pow(distance, 2.0)));

        acceleration[i] = v01.normalize(scale) + acceleration[i];
        acceleration[j] = -acceleration[i];
      }
    }

    for (auto& a : acceleration) {
      if (a.x > limit2) a.x = limit2;
      if (a.x < -limit2) a.x = -limit2;
      if (a.y > limit2) a.y = limit2;
      if (a.y < -limit2) a.y = -limit2;
      if (a.z > limit) a.z = limit;
      if (a.z < -limit) a.z = -limit;
    }
    
    // drag (slows things down)
    for (int i = 0; i < velocity.size(); i++) {
      acceleration[i] -= velocity[i] * 0.1;
      //a[i] = a[i] - velocity[i] * 0.1
    }

  
    // Integration
    //
    vector<Vec3f> &position(mesh.vertices());
    for (int i = 0; i < velocity.size(); i++) {
      // "semi-implicit" Euler integration
      velocity[i] += acceleration[i] / mass[i] * dt;
      //cout << acceleration[1] << endl;
      //v[i] = v[i] + a[i]/m[i] * t
      position[i] += velocity[i] * dt;

      // Explicit (or "forward") Euler integration would look like this:
      // position[i] += velocity[i] * dt;
      // velocity[i] += acceleration[i] / mass[i] * dt;
    }

    // clear all accelerations (IMPORTANT!!)
    for (auto &a : acceleration) a.zero();
  }

  bool onKeyDown(const Keyboard &k) override {
    if (k.key() == 'i') {
      Vec3f sum(0, 0, 0);
      for (int i = 0; i < velocity.size(); i++) {
        sum += mesh.vertices()[i];
      }
      sum /= velocity.size();
      nav().pos(sum);
    }

    if (k.key() == ' ') {
      freeze = !freeze;
    }

    if (k.key() == '1') {
      // introduce some "random" forces
      for (int i = 0; i < velocity.size(); i++) {
        // F = ma
        //a = F/m
        //acceleration = randomVec / mass[i]
        acceleration[i] = randomVec3f(5) / mass[i];
      }
    }

    return true;
  }

  void onDraw(Graphics &g) override {
    g.clear(0.3);
    g.shader(pointShader);
    g.shader().uniform("pointSize", pointSize / 100);
    g.blending(true);
    g.blendTrans();
    g.depthTesting(true);
    g.draw(mesh);
  }
};

int main() {
  AlloApp app;
  app.configureAudio(48000, 512, 2, 0);
  app.start();
}

string slurp(string fileName) {
  fstream file(fileName);
  string returnValue = "";
  while (file.good()) {
    string line;
    getline(file, line);
    returnValue += line + "\n";
  }
  return returnValue;
}