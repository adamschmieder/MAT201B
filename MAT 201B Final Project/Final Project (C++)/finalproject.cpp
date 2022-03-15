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
#include "al/app/al_GUIDomain.hpp"

using namespace al;
using namespace std;

string slurp(string fileName); // forward declaration

class MyApp : public App
{
public:
  int pics = 14;
  Mesh pic[14], rgb[14], hsv[14], somethingElse[14];
  Mesh actual, previous, current[14];
  HSV hueSatVal[14];
  Color col[14];

  Parameter zScale{"zScale", 1.0, 0.00, 10.0};
  Parameter pointSize{"pointSize", "", 0.15, "", 0.01, 0.5};
  Parameter rotation{"rotation", 0, -35.0, 35.0};
  // switch to parameter OSC

  const char *filename[14];
  Image imageData[14];
  int k = 0;
  // the current mesh

  int meshType = 1;
  float bVal = 1.0;
  float iVal = 1.0;

  ShaderProgram pointShader;

  //double rotation{0};

  void onInit() override
  {
    auto GUIdomain = GUIDomain::enableGUI(defaultWindowDomain());
    auto &gui = GUIdomain->newGUI();
    gui.add(zScale);
    gui.add(pointSize);
    gui.add(rotation);
    parameterServer() << zScale;
    parameterServer() << rotation;
  }

  void onCreate() override
  {

    // pointShader.compile(slurp("../point-vertex.glsl"),
    //                     slurp("../point-fragment.glsl"),
    //                     slurp("../point-geometry.glsl"));
    if (!(pointShader.compile(slurp("../point-vertex.glsl"),
                              slurp("../point-fragment.glsl"),
                              slurp("../point-geometry.glsl"))))
    {
      cout << "shader didn't compile" << endl;
      exit(1);
    }
    filename[0] = "monstrous500.jpeg";
    filename[1] = "zaborsky500.jpeg";
    filename[2] = "lightoftheworld500.jpeg";
    filename[3] = "dove500.jpeg";
    filename[4] = "baptism500.jpeg";
    filename[5] = "pray500.jpeg";
    filename[6] = "sacrifice500.jpeg";
    filename[7] = "love500.jpeg";
    filename[8] = "sun500.jpeg";
    filename[9] = "peacekeeper500.jpeg";
    filename[10] = "quality500.jpeg";
    filename[11] = "excess500.jpeg";
    filename[12] = "urgency500.jpeg";
    filename[13] = "soilflow500.jpeg";

    for (int p = 0; p < pics; p++)
    {
      imageData[p] = Image(filename[p]);
      if (imageData[p].array().size() == 0)
      {
        cout << "failed to load image " << p << endl;
        //exit(1);
      }
      cout << "loaded image size: " << imageData[p].width() << ", " << imageData[p].height() << endl;
      int W = imageData[p].width();
      cout << W << endl;
      int H = imageData[p].height();
      cout << H << endl;

      pic[p].primitive(Mesh::POINTS);
      rgb[p].primitive(Mesh::POINTS);
      hsv[p].primitive(Mesh::POINTS);
      somethingElse[p].primitive(Mesh::POINTS);

      // make points like assignment 2
      // can Ribbonize with LINES
      // check out tangle-mesh.cpp
      // LINE_STRIP looks terrible

      actual.primitive(Mesh::POINTS);
      previous.primitive(Mesh::POINTS);
      for (int b = 0; b < pics; b++)
      {
        current[b].primitive(Mesh::POINTS);
      }

      // iterate through all the pixel, scanning each row
      for (int row = 0; row < H; row++)
      {
        for (int column = 0; column < W; column++)
        {
          auto pixel = imageData[p].at(column, H - row - 1);
          pic[p].vertex(1.0 * column / W, 1.0 * row / H, 0.0);
          pic[p].color(pixel.r / 255.0, pixel.g / 255.0, pixel.b / 255.0);

          rgb[p].vertex((-1.0 * pixel.r / 255.0) + 1.0, pixel.g / 255.0, pixel.b / 255.0);
          rgb[p].color(pixel.r / 255.0, pixel.g / 255.0, pixel.b / 255.0);

          col[p].r = pixel.r / 255.0;
          col[p].g = pixel.g / 255.0;
          col[p].b = pixel.b / 255.0;

          hueSatVal[p].operator=(col[p]);
          //hueSatVal.s = p, hueSatVal.h = azimuth, hueSatVal.v = height; multiply azimuth by twopi
          hsv[p].vertex(((hueSatVal[p].s * cos(hueSatVal[p].h * M_2PI)) / 2.0) + 0.5, (hueSatVal[p].v * 1.1), ((hueSatVal[p].s * sin(hueSatVal[p].h * M_2PI))) / 2.0);
          hsv[p].color(hueSatVal[p]);

          somethingElse[p].vertex(1.0 * column / W, 1.0 * row / H, (pixel.b / 255.0) * 2.0);
          somethingElse[p].color(pixel.r / 255.0, pixel.g / 255.0, pixel.b / 255.0);
        }
      }
    }
    actual = pic[0];
    current[0] = actual;
    for (int b = 1; b < pics; b++)
    {
      current[b] = pic[b];
    }
    previous = actual;
    nav().pos(0.5, 0.5, 3.5);
  }

  float t = 0;
  void onAnimate(double dt) override
  {
    // = angle + 0.1;
    t = dt + t;
    for (int i = 0; i < (current[k].vertices().size()); i++)
    {
      current[k].vertices()[i] = (previous.vertices()[i] * (1 - (t * iVal) / 2.0)) + (actual.vertices()[i] * (t * iVal) / 2.0);
      // crashes cause all different sizes
      current[k].vertices()[i].z += (current[k].colors()[i].luminance() * zScale);

      // current[k].colors()[i].luminance()
    }
  }

  void onMessage(osc::Message &m) override
  {
    if (m.addressPattern() == "/picType")
    {
      m >> k;
      t = 0;
    }
    if (m.addressPattern() == "/meshType")
    {
      m >> meshType;
      switch (meshType)
      {
      case 1:
        previous = actual;
        actual = pic[k];
        t = 0;
        break;
      case 4:
        previous = actual;
        actual = rgb[k];
        t = 0;
        break;
      case 3:
        previous = actual;
        actual = hsv[k];
        t = 0;
        break;
      case 2:
        previous = actual;
        actual = somethingElse[k];
        t = 0;
        break;
      default:
        break;
      }
    }
    if (m.addressPattern() == "/interpVal")
    {
      m >> iVal;
    }
  }

  void onDraw(Graphics &g) override
  {
    g.clear(0.0f);
    g.shader(pointShader);
    g.shader().uniform("pointSize", pointSize / 100);
    g.pointSize(1.5);

    //g.blending(true);
    //g.blendTrans();
    g.depthTesting(true);

    //g.meshColor();
    g.rotate(rotation, (Vec3f(0, 1, 0)));
    g.draw(current[k]);
    // point shader not working
    // also crashing
    // make image 500x500
  }
};

int main()
{
  MyApp app;
  app.configureAudio(48000, 512, 2, 0);
  app.start();
}

string slurp(string fileName)
{
  fstream file(fileName);
  string returnValue = "";
  while (file.good())
  {
    string line;
    getline(file, line);
    returnValue += line + "\n";
  }
  return returnValue;
}