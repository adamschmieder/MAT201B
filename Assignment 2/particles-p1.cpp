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
  Parameter pointSize{"/pointSize", "", 5.0, "", 0.0, 5.0};
  Parameter timeStep{"/timeStep", "", 0.1, "", 0.01, 0.6};
  Parameter gravConstant{"/gravConstant", "", 0.1, "", 0.0, 5.0};
  //

  ShaderProgram pointShader;

  //  simulation state
  Mesh mesh;  // position *is inside the mesh* mesh.vertices() are the positions
  vector<Vec3f> velocity;
  vector<Vec3f> acceleration;
  vector<float> mass;
  int particles = 50;
  

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
  float limit = 2.0;
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

  
    //distance = sqrt(
              //pow((mesh.vertices()[1].x - mesh.vertices()[0].x), 2.0) +
              //pow((mesh.vertices()[1].y - mesh.vertices()[0].y), 2.0) +
              //pow((mesh.vertices()[1].z - mesh.vertices()[0].z), 2.0));

    //Vec3f v01 = mesh.vertices()[1] - mesh.vertices()[0];
    //cout << distance << endl;

    //distance2 = (mesh.vertices()[1] - mesh.vertices()[0]).mag();
    //cout << distance2 << endl;

    //cout << v01 << endl;
    //double distance = v01.mag();
    //cout << distance << endl;
    //cout << v01.normalize() << endl;

    //acceleration[0] = v01.normalize(scale);
    //acceleration[1] = -acceleration[0];


    for (int i = 0; i < particles; i++) {
      for (int j = i + 1; j < particles; j++){
        //acceleration[i] = acceleration[i] + acceleration[j];
        v01 = mesh.vertices()[j] - mesh.vertices()[i];
        distance = (mesh.vertices()[j] - mesh.vertices()[i]).mag();
        scale = (gravConstant/(pow(distance, 2.0)));

        acceleration[i] = v01.normalize(scale) + acceleration[i];

        acceleration[j] = -acceleration[i] + acceleration[j];
      }
    }

    for (auto& a : acceleration) {
      //cout << a.mag() << endl;
      if (a.x > limit)
        a.x = limit;
      if (a.x < -limit)
        a.x = -limit;
      if (a.y > limit)
        a.y = limit;
      if (a.y < -limit)
        a.y = -limit;
      if (a.z > limit)
        a.z = limit;
      if (a.z < -limit)
        a.z = -limit;
      //cout << a << endl;
    }
    //cout << acceleration[0].mag() << endl;
    //cout << acceleration[0].normalize() << endl;

    //if(acceleration[0].mag() > 10.0)

    // how do i get the particles to go close to eachother??? 
    // mesh.vertices()[i] is how u get particle positions

    // XXX you put code here that calculates gravitational forces and sets accelerations
    // These are pair-wise. Each unique pairing of two particles
    // These are equal but opposite: A exerts a force on B while B exerts that
    // same amount of force on A (but in the opposite direction!) Use a nested
    // for loop to visit each pair once The time complexity is O(n*n)
    //
      // Vec3f has lots of operations you might use...
    // ??? +=
    // ??? -=
    // ??? +
    // ??? -
    // ??? .normalize() ~ Vec3f points in the direction as it did, but has length 1
    // ??? .normalize(float scale) ~ same but length `scale`
    // ??? .mag() ~ length of the Vec3f
    // ??? .magSqr() ~ squared length of the Vec3f
    // ??? .dot(Vec3f f)
    // ??? .cross(Vec3f f)


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