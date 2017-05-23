#include "terrain.hpp"
#include "draw.h"
#include "image_io.h"
#include "pngUtilities.hpp"
#include "vec.h"

#include "program.h"
#include "uniforms.h"

#include <iostream>
#include <limits.h>
#include <unistd.h>

std::string getCurrentPath(){
  char result[ PATH_MAX ];
  ssize_t count = readlink( "/proc/self/exe", result, PATH_MAX );
  std::string current_bin = std::string( result, (count > 0) ? count : 0 );
  std::size_t found = current_bin.find_last_of("/\\");
  std::string current_path = current_bin.substr(0,found);
  return current_path;
}

// Generated Terrain
GeneratedTerrain::GeneratedTerrain() : mesh_(GL_TRIANGLES){
  PngUtilities png;
  std::string pathToMap = getCurrentPath() + "/../circuit.png";
  png.init(pathToMap.c_str());
  unsigned int png_height = png.getHeight();
  unsigned int png_width = png.getWidth();
  step = 2; // 1 point sur 2 de l'image png est pris en compte dans la génération de terrain (pour lisser un peu le terrain)

  Transform transform = RotationX(90) * Scale(100,20,100);
  height = png_height/step;
  width = png_width/step;
  std::cout << height << std::endl;
  std::cout << width << std::endl;

  // creation of the terrain grid (vertex and texture)
  std::vector< std::vector< Vector > > vVertexData(height, std::vector< Vector >(width)); 
  std::vector< std::vector< vec2 > > vCoordsData(height, std::vector< vec2 >(width)); 

  // size of the texture
  float fTextureU = float(width) * 0.1f;
  float fTextureV = float(height) * 0.1f;

  for(unsigned int i = 0;i < height;i++) {
    for(unsigned int j = 0;j < width;j++) {
      // scale of height and width
      float fScaleH = float(i) / float(height - 1);
      float fScaleW = float(j) / float(width - 1);
      // vertew height in y between 0 and 1
      float fVertexHeight = png.getValue(i*step, j*step) / 255.0f;
      // add the coordinates of each vertex
      vVertexData[i][j] = transform(Vector(-0.5 + fScaleW, fVertexHeight, -0.5 + fScaleH));
      vCoordsData[i][j] = vec2(fTextureU * fScaleW, fTextureV * fScaleH);
    }
  }
  
  // lissage du terrain
  // on fait une moyenne pondérée de la hauteur de chaque sommet en fonction de la hauteur de ses voisins, avec 20 itérations
  smooth(vVertexData, 20);

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


  //Transform rotation = RotationX(90);
  //Transform scale = Scale(100, 100, 20);
  //Vector scale = Vector(100,20,100);
  // create the mesh
  for(unsigned int i = 0;i < height;i++) {
    for(unsigned int j = 0;j < width;j++) {
      // add the normal for each vertex
      mesh_.normal(vFinalNormals[i][j]);
      // add the texture
      mesh_.texcoord(vCoordsData[i][j]);
      // add the vertex
      //Point unscaled_rotated_vertex = rotation((Point)vVertexData[i][j]);
      //Point scaled_rotated_vertex = scale(unscaled_rotated_vertex);
      //mesh_.vertex(scaled_rotated_vertex);
      //Point unscaled_vertex = (Point)vVertexData[i][j];
      //mesh_.vertex(Point(unscaled_vertex.x * scale.x, unscaled_vertex.y * scale.y, unscaled_vertex.z * scale.z));
      mesh_.vertex((Point)vVertexData[i][j]);
    }
  }


  // build the triangles
  for(unsigned int i = 0;i < height - 1;i++) {
    for(unsigned int j = 0;j < width - 1;j++) {
      // create 2 triangles for each quad
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

void GeneratedTerrain::smooth(std::vector< std::vector< Vector > >& vVertexData, const unsigned int iterations) {
  unsigned int nb_iterations = 0;
  while(nb_iterations < iterations) {
    // lissage du terrain
    unsigned int id = 0;
    std::vector< std::vector< float > > neighbours;
    neighbours.resize(2);
    for(unsigned int i = 0;i < height;i++) {
      for(unsigned int j = 0;j < width;j++) {
        // voisins directs dans neighbours[0], voisins en diagonales dans neighbours[1]
        if(i > 0) {
          neighbours[0].push_back(vVertexData[i-1][j].z); // up
          if(j > 0) {
            neighbours[1].push_back(vVertexData[i-1][j-1].z); // up-left
          }
          if(j < width) {
            neighbours[1].push_back(vVertexData[i-1][j+1].z); // up-right
          }
        }
        if(i < height-1) {
          neighbours[0].push_back(vVertexData[i+1][j].z); // down
          if(j > 0) {
            neighbours[1].push_back(vVertexData[i+1][j-1].z); // down-left
          }
          if(j < width) {
            neighbours[1].push_back(vVertexData[i+1][j+1].z); // down-right
          }
        }
        if(j > 0) {
          neighbours[0].push_back(vVertexData[i][j-1].z); // left        
        }
        if(j < width-1) {
          neighbours[0].push_back(vVertexData[i][j+1].z); // right
        }

        // calcul moyenne (0.5 * sommet initial.y + 0.3 * sommets_directs.y + 0.2 * sommets_diagonales.y)
        float z_average = 0.5 * vVertexData[i][j].z;

        float sum = 0;
        for(unsigned int k = 0;k < neighbours[0].size();k++) {
          sum += neighbours[0][k]; 
        }
        z_average += 0.3 * (sum / neighbours[0].size()); // sommets directs

        sum = 0;
        for(unsigned int k = 0;k < neighbours[1].size();k++) {
          sum += neighbours[1][k];
        }
        z_average += 0.2 * (sum / neighbours[1].size()); // sommets diagonales

        // maj du sommet avec la moyenne en y
        vVertexData[i][j].z = z_average;

        id++;
        neighbours[0].clear();
        neighbours[1].clear();
      }
    }

    nb_iterations++;
  }
}

void GeneratedTerrain::project(const Point& from, Point& to, Vector& n) const {
  to.z = 0 ;
  n = Vector(0.f, 0.f, 1.f) ;
}

void GeneratedTerrain::draw(const Transform& v, const Transform& p) {
  ::draw(mesh_, RotationX(90) * Scale(100,10,100), v, p) ;
}

void GeneratedTerrain::draw(const GLuint& shaders_program, Transform model, Transform view, Transform proj) {
  // configurer le pipeline 
  glUseProgram(shaders_program);

  // configurer le shader program
  // . composer les transformations : model, view et projection
  Transform mv = view * model;
  Transform mvp = proj * view * model;

  // . parametrer le shader program :
  //   . transformation : la matrice declaree dans le vertex shader s'appelle mvpMatrix
  program_uniform(shaders_program, "mvMatrix", mv);        
  program_uniform(shaders_program, "normalMatrix", mv.normal());        
  program_uniform(shaders_program, "mvpMatrix", mvp);
  program_uniform(shaders_program, "modelMatrix", model);
  program_uniform(shaders_program, "viewMatrix", view);
  program_uniform(shaders_program, "viewInvMatrix", view.inverse());

  // program_use_texture(shaders_program, "texture0", 0, textures[0]);
  // program_use_texture(shaders_program, "texture1", 0, textures[1]);
  // program_use_texture(shaders_program, "texture2", 0, textures[2]);


  //terrain_.draw(m_camera.view(), m_camera.projection(window_width(), window_height(), 45.f)) ;
  //generatedTerrain_.draw(m_camera.view(), m_camera.projection(window_width(), window_height(), 45.f)) ;
  mesh_.draw(shaders_program);
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
