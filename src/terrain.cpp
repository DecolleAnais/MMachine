#include "terrain.hpp"
#include "draw.h"
#include "image_io.h"
#include "pngUtilities.hpp"
#include "vec.h"
#include <iostream>


// Generated Terrain
GeneratedTerrain::GeneratedTerrain() {
  PngUtilities png;
  png.init("/home/ad/Documents/synthese_image/gkit2light/MMachine/circuit.png");
  height = png.getHeight();
  width = png.getWidth();

  std::vector< std::vector< Vector > > vVertexData(height, std::vector< Vector >(width)); 
  for(unsigned int i;i < height;i++) {
    for(unsigned int j;j < width;j++) {
      float fScaleH = float(i) / float(height - 1);
      float fScaleW = float(j) / float(width - 1);
      float fVertexHeight = png.getValue(i, j) / 255.0f;
      vVertexData[i][j] = Vector(-0.5 + fScaleW, fVertexHeight, -0.5 + fScaleH);
      std::cout << "p(" << vVertexData[i][j].x << "," << vVertexData[i][j].y << "," << vVertexData[i][j].z << ")" << std::endl;
    }
  }

  


  png.free();
}

void GeneratedTerrain::project(const Point& from, Point& to, Vector& n) const {

}

void GeneratedTerrain::draw(const Transform& v, const Transform& p) {
  ::draw(mesh_, Identity(), v, p) ;
}


//Flat Terrain

FlatTerrain::FlatTerrain(const Point& pmin, const Point& pmax) : mesh_(GL_TRIANGLES) {
  //terrain mesh
  unsigned int a = mesh_.vertex(pmin.x, pmin.y, 0.f) ;
  unsigned int b = mesh_.vertex(pmax.x, pmin.y, 0.f) ;
  unsigned int c = mesh_.vertex(pmax.x, pmax.y, 0.f) ;
  unsigned int d = mesh_.vertex(pmin.x, pmax.y, 0.f) ;

  mesh_.triangle(a, b, c) ;
  mesh_.triangle(a, c, d) ;
}

void FlatTerrain::project(const Point& from, Point& to, Vector& n) const {
  to.z = 0 ;
  n = Vector(0.f, 0.f, 1.f) ;
}

void FlatTerrain::draw(const Transform& v, const Transform& p) {
  ::draw(mesh_, Identity(), v, p) ;
}
