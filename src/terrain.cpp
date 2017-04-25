#include "terrain.hpp"
#include "draw.h"
#include "image_io.h"
#include "pngUtilities.hpp"
#include "vec.h"
#include <iostream>


// Generated Terrain
GeneratedTerrain::GeneratedTerrain() : mesh_(GL_TRIANGLES){
  PngUtilities png;
  png.init("/home/ad/Documents/synthese_image/gkit2light/MMachine/circuit.png");
  height = png.getHeight();
  width = png.getWidth();

  // creation of the terrain grid
  std::vector< std::vector< Vector > > vVertexData(height, std::vector< Vector >(width)); 
  for(unsigned int i = 0;i < height;i++) {
    for(unsigned int j = 0;j < width;j++) {
      // scale of height and width
      float fScaleH = float(i) / float(height - 1);
      float fScaleW = float(j) / float(width - 1);
      // vertew height in y between 0 and 1
      float fVertexHeight = png.getValue(i, j) / 255.0f;
      // add the coordinates of each vertex
      vVertexData[i][j] = Vector(-0.5 + fScaleW, fVertexHeight, -0.5 + fScaleH);
    }
  }

  // calculation of the triangles and their normal

  // array of arrays of normalized normals
  // 2 rows = 1 for the normals of the triangles0 (half of quad) and 1 for the triangles1 (the other half)
  std::vector< std::vector < Vector > > vNormals[2];
  for(unsigned int i = 0;i < 2;i++) {
    vNormals[i] = std::vector < std::vector < Vector > > (height - 1, std::vector< Vector >(width - 1));
  }  
  // scan each vertex data (each quad)
  for(unsigned int i = 0;i < height - 1;i++) {
    for(unsigned int j = 0;j < width - 1;j++) {
      // create 2 triangles for each quad
      Vector vTriangle0[] = {vVertexData[i][j], vVertexData[i + 1][j], vVertexData[i + 1][j + 1]};
      Vector vTriangle1[] = {vVertexData[i + 1][j + 1], vVertexData[i][j + 1], vVertexData[i][j]};

      // calculation of the normals
      Vector vTriangleNorm0 = cross(vTriangle0[0] - vTriangle0[1], vTriangle0[1] - vTriangle0[2]);
      Vector vTriangleNorm1 = cross(vTriangle1[0] - vTriangle1[1], vTriangle1[1] - vTriangle1[2]);

      // stock the normalized normals
      vNormals[0][i][j] = normalize(vTriangleNorm0);
      vNormals[1][i][j] = normalize(vTriangleNorm1);
    }
  }

  // calculation of the final normal for each quad
  std::vector< std::vector< Vector > > vFinalNormals = std::vector< std::vector< Vector > > (height, std::vector< Vector >(width));
  for(unsigned int i = 0;i < height;i++) {
    for(unsigned int j = 0;j < width;j++) {

      // the final normal of a vertex is defined by the sum of the normals of the triangles around it
      Vector vFinalNormal = Vector(0.0f, 0.0f, 0.0f);

      // upper-left triangles
      if(i != 0 && j != 0) {
        for(unsigned int k = 0;k < 2;k++) {
          vFinalNormal = vFinalNormal + vNormals[k][i - 1][j - 1];
        }
      }

      // upper-right triangles
      if(i != 0 && j != (width - 1)) {
          vFinalNormal = vFinalNormal + vNormals[0][i - 1][j];
      }

      // bottom-right triangles
      if(i != (height - 1) && j != (width - 1)) {
        for(unsigned int k = 0;k < 2;k++) {
          vFinalNormal = vFinalNormal + vNormals[k][i][j];
        }
      }

      // bottom-left triangles
      if(i != (height - 1) && j != 0) {
          vFinalNormal = vFinalNormal + vNormals[1][i][j - 1];
      }

      // stock the final  normalized normal of the vertex
      vFinalNormal = normalize(vFinalNormal);
      vFinalNormals[i][j] = vFinalNormal;
    }
  }

  // create the mesh
  for(unsigned int i = 0;i < height;i++) {
    for(unsigned int j = 0;j < width;j++) {
      // add the normal for each vertex
      mesh_.normal(vFinalNormals[i][j]);
      // add the vertex
      mesh_.vertex((Point)vVertexData[i][j]);
    }
  }

  // build the triangles
  for(unsigned int i = 0;i < height - 1;i++) {
    for(unsigned int j = 0;j < width - 1;j++) {
      // create 2 triangles for each quad
      //mesh_.triangle(vVertexData[i][j], vVertexData[i + 1][j], vVertexData[i + 1][j + 1]);
      //mesh_.triangle(vVertexData[i + 1][j + 1], vVertexData[i][j + 1], vVertexData[i][j]);
      mesh_.triangle((i + 1) * width + j,
                      i * width + j,
                      (i + 1) * width + (j + 1));
      mesh_.triangle(i * width + j,
                    (i + 1) * width + (j + 1),
                    i * width + (j + 1));
    }
  }

  png.free();
}

void GeneratedTerrain::project(const Point& from, Point& to, Vector& n) const {
  to.z = 0 ;
  n = Vector(0.f, 0.f, 1.f) ;
}

void GeneratedTerrain::draw(const Transform& v, const Transform& p) {
  ::draw(mesh_, RotationX(90) * Scale(30,10,30), v, p) ;
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