/*
  This example shows how to use Image, Array and Texture to read a .jpg file,
display it as an OpenGL texture and print the pixel values on the command line.
Notice that while the intput image has only 4 pixels, the rendered texture is
smooth.  This is because interpolation is done on the GPU.
  Karl Yerkes and Matt Wright (2011/10/10)
*/

#include <cassert>
#include <cstdint>
#include <iostream>
#include <vector>

#include "al/app/al_App.hpp"
#include "al/graphics/al_Image.hpp"

using namespace al;
using namespace std;

class MyApp : public App {
 public:
  Mesh pic, rgb, hsv, somethingElse;
  Mesh actual;
  Mesh previous;
  Mesh current;
  HSV hueSatVal;
  Color col;
  

  void onCreate() override {
    const char *filename = "zaborsky5.jpeg";
    auto imageData = Image(filename);
    if (imageData.array().size() == 0) {
      cout << "failed to load image" << endl;
      exit(1);
    }
    cout << "loaded image size: " << imageData.width() << ", "
         << imageData.height() << endl;

    int W = imageData.width();
    cout << W << endl;
    int H = imageData.height();
    cout << H << endl;

    pic.primitive(Mesh::POINTS);
    actual.primitive(Mesh::POINTS);
    rgb.primitive(Mesh::POINTS);
    hsv.primitive(Mesh::POINTS);
    //somethingElse.primitive(Mesh::LINE_LOOP);
    somethingElse.primitive(Mesh::TRIANGLES);
    previous.primitive(Mesh::POINTS);
    current.primitive(Mesh::POINTS);


    // iterate through all the pixel, scanning each row
    for (int row = 0; row < H; row++) {
      for (int column = 0; column < W; column++) {
        auto pixel = imageData.at(column, H - row - 1);
        pic.vertex(1.0 * column / W, 1.0 * row / H, 0.0);
        pic.color(pixel.r / 255.0, pixel.g / 255.0, pixel.b / 255.0);

        // initial setting of actual
        actual.vertex(1.0 * column / W, 1.0 * row / H, 0.0);
        actual.color(pixel.r / 255.0, pixel.g / 255.0, pixel.b / 255.0);

        rgb.vertex((-1.0 * pixel.r/255.0) + 1.0, pixel.g/255.0, pixel.b/255.0);
        rgb.color(pixel.r / 255.0, pixel.g / 255.0, pixel.b / 255.0);

        col.r = pixel.r/255.0;
        col.g = pixel.g/255.0;
        col.b = pixel.b/255.0;
        //cout << col.r << endl;
        hueSatVal.operator=(col);
        //cout << hueSatVal.h << endl;

        //hueSatVal.s = p
        //hueSatVal.h = azimuth
        //hueSatVal.v = height
        //multiply azimuth by twopi
        
        hsv.vertex(((hueSatVal.s * cos(hueSatVal.h * M_2PI))/2.0)+0.5, (hueSatVal.v * 1.1), ((hueSatVal.s * sin(hueSatVal.h * M_2PI)))/2.0);
        hsv.color(hueSatVal);

        somethingElse.vertex(1.0 * column / W, 1.0 * row / H, (pixel.b/255.0)*2.0);
        somethingElse.color(pixel.r / 255.0, pixel.g / 255.0, pixel.b / 255.0);
      }
    }
    current = actual;
    actual = pic;
    previous = actual;

    // set the camera position back some (z=3) and center on (x=0.5, y=0.5)
    //actual.scale(2);
    nav().pos(0.5, 0.5, 3);
  }

  float t = 0;
  void onAnimate(double dt) override {
    t = dt+t;
    //cout << t << endl;

    //mix = a * (1 - p) + b * p
    //where p is a parameter within the interval (0, 1)
    //a and b are numbers or arrays of numbers (vertices)

    if (t <= 1) {
      //cout << "test" << endl;
      for(int i = 0; i < (current.vertices().size()); i++){
        current.vertices()[i] = (previous.vertices()[i] * (1 - t)) + (actual.vertices()[i] * t);
      }
    }
  }

  bool onKeyDown(const Keyboard &k) override {
    switch (k.key()) {
      case '1':
        previous = actual;
        actual = pic;
        t = 0;
        break;
      case '2':
        previous = actual;
        actual = rgb;
        t = 0;
        break;
      case '3':
        previous = actual;
        actual = hsv;
        t = 0;
        break;
      case '4':
        previous = actual;
        actual = somethingElse;
        t = 0;
        break;

      default:
        break;
    }
    return true;
  }

  void onDraw(Graphics &g) override {
    g.clear(0.2f);
    g.meshColor();
    g.draw(current);
    
  }
};

int main() {
  MyApp app;
  app.configureAudio(48000, 512, 2, 0);
  app.start();
}