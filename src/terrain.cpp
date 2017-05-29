#include "terrain.hpp"
#include "parser.hpp"
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
GeneratedTerrain::GeneratedTerrain(const Point& pmin, const Point& pmax) : mesh_(GL_TRIANGLES){
  PngUtilities png;
  std::string pathToMap = getCurrentPath() + "/../data/circuit.png";
  png.init(pathToMap.c_str());
  unsigned int png_height = png.getHeight();
  unsigned int png_width = png.getWidth();
  step = 2; // 1 point sur 2 de l'image png est pris en compte dans la génération de terrain (pour lisser un peu le terrain)

  Transform transform = Scale(200,200,30);
  height = png_height/step;
  width = png_width/step;

  // creation of the terrain grid (vertex and texture)
  std::vector< std::vector< Vector > > vVertexData(width, std::vector< Vector >(height)); 
  std::vector< std::vector< vec2 > > vCoordsData(width, std::vector< vec2 >(height)); 

  // size of the texture
  float fTextureU = float(png_height);// * 0.1f;
  float fTextureV = float(png_width);// * 0.1f;

  for(unsigned int i = 0;i < width;i++) {
    for(unsigned int j = 0;j < height;j++) {
      // scale of width and height
      float fScaleH = float(i) / float(width - 1);
      float fScaleW = float(j) / float(height - 1);
      // vertex altitude between 0 and 1
      float fVertexHeight = png.getValue(i*step, (height - j - 1)*step) / 255.0f;
      // add the coordinates of each vertex
      vVertexData[i][j] = transform(Vector(/*-0.5 +*/ fScaleH, /*-0.5 +*/ fScaleW, fVertexHeight));
      vCoordsData[i][j] = vec2(/*fTextureU **/ fScaleH , /*fTextureV **/ fScaleW);
      //std::cout << vCoordsData[i][j].x << ";" << vCoordsData[i][j].y << std::endl;
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
    vNormals[i] = std::vector < std::vector < Vector > > (width - 1, std::vector< Vector >(height - 1));
  }  
  // scan each vertex data (each quad)
  for(unsigned int i = 0;i < width - 1;i++) {
    for(unsigned int j = 0;j < height - 1;j++) {
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
  std::vector< std::vector< Vector > > vFinalNormals = std::vector< std::vector< Vector > > (width, std::vector< Vector >(height));
  for(unsigned int i = 0;i < width;i++) {
    for(unsigned int j = 0;j < height;j++) {

      // the final normal of a vertex is defined by the sum of the normals of the triangles around it
      Vector vFinalNormal = Vector(0.0f, 0.0f, 0.0f);

      // upper-left triangles
      if(i != 0 && j != 0) {
        for(unsigned int k = 0;k < 2;k++) {
          vFinalNormal = vFinalNormal + vNormals[k][i - 1][j - 1];
        }
      }

      // upper-right triangles
      if(i != 0 && j != (height - 1)) {
          vFinalNormal = vFinalNormal + vNormals[0][i - 1][j];
      }

      // bottom-right triangles
      if(i != (width - 1) && j != (height - 1)) {
        for(unsigned int k = 0;k < 2;k++) {
          vFinalNormal = vFinalNormal + vNormals[k][i][j];
        }
      }

      // bottom-left triangles
      if(i != (width - 1) && j != 0) {
          vFinalNormal = vFinalNormal + vNormals[1][i][j - 1];
      }

      // stock the final  normalized normal of the vertex
      vFinalNormal = normalize(vFinalNormal);
      vFinalNormals[i][j] = vFinalNormal;
    }
  }

  // create the mesh
  for(unsigned int i = 0;i < width;i++) {
    for(unsigned int j = 0;j < height;j++) {
      // add the normal for each vertex
      mesh_.normal(vFinalNormals[i][j]);
      // add the texture
      mesh_.texcoord(vCoordsData[i][j]);
      // add the vertex
      mesh_.vertex((Point)vVertexData[i][j]);
    }
  }


  // build the triangles
  for(unsigned int i = 0;i < width - 1;i++) {
    for(unsigned int j = 0;j < height - 1;j++) {
      // create 2 triangles for each quad
      mesh_.triangle((i + 1) * height + j,
                      (i + 1) * height + (j + 1),
                      i * height + j);
      mesh_.triangle(i * height + j,
                    (i + 1) * height + (j + 1),
                    i * height + (j + 1));
    }
  }

  // initialize the checkpoints on the terrain
  setCheckpoints(transform);

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
  float step = mesh_.positions().at(1).y - mesh_.positions().at(0).y;

  // std::cout << Point(mesh_.positions().at(1)) << " ; " << Point(mesh_.positions().at(0)) << std::endl;
  // std::cout << height << " " << width << "; " << step << std::endl;

  unsigned int girdX = to.x / step;
  unsigned int girdY = to.y / step;
  unsigned int vertexTopIndex = girdX * width + girdY;
  unsigned int vertexBottomIndex = (girdX+1) * width + girdY;

  // std::cout << girdX << " " << girdY << "; " << vertexTopIndex << " " << vertexBottomIndex << std::endl;
  // std::cout << "VOITURE : " << to << std::endl; 
  // std::cout << Point(mesh_.positions().at(0)) << " ; " << Point(mesh_.positions().at((width-1) * (height-1))) << std::endl;

  if(vertexBottomIndex >= height * width || vertexTopIndex >= height * width 
        || vertexTopIndex+1 >= height * width || vertexBottomIndex+1 >= height * width){
    //std::cerr << "***** ERROR : Player not on terrain *****" << std::endl;
    n = Vector(0.f,0.f,1.f);
    return;
  }

  unsigned int tl = vertexTopIndex;
  unsigned int tr = vertexTopIndex + 1;
  unsigned int bl = vertexBottomIndex;
  unsigned int br = vertexBottomIndex + 1;

  // std::cout << tl << "\t\t\t" << tr << std::endl; 
  // std::cout << "\t\t\t" << to << std::endl;
  // std::cout << bl << "\t\t\t" << br << std::endl;

  float slip = 0.05;
  float slipCosAngle = 0.9;
  if(((GeneratedTerrain *)this)->collideWithTriangleGird(to, tl, bl, tr)){
    to.z = ((GeneratedTerrain *)this)->getHeight(to, tl, bl, tr);
    n = ((GeneratedTerrain *)this)->getNormal(to, tl, bl, tr);

    // Glisade en pente
    if(dot(n, Vector(0.f, 0.f, 1.f)) < slipCosAngle){
      to.x += n.x * slip;
      to.y += n.y * slip;
      n = ((GeneratedTerrain *)this)->getNormal(to, tl, bl, tr);
    }
  }
  else if(((GeneratedTerrain *)this)->collideWithTriangleGird(to, tr, bl, br)){
    to.z = ((GeneratedTerrain *)this)->getHeight(to, tr, bl, br);
    n = ((GeneratedTerrain *)this)->getNormal(to, tr, bl, br);

    // Glissade en pente
    if(dot(n, Vector(0.f, 0.f, 1.f)) < slipCosAngle){
      to.x += n.x * slip;
      to.y += n.y * slip;
      n = ((GeneratedTerrain *)this)->getNormal(to, tl, bl, tr);
    }
  }
  else {
    to.z = 0.f;
    n = Vector(0.f,0.f,1.f);
    //std::cerr << "***** ERROR : Terrain projection failed! *****" << std::endl;
  }
}

bool GeneratedTerrain::collideWithTriangleGird(Point pos, int ia, int ib, int ic){
  Point a(mesh_.positions().at(ia));
  Point b(mesh_.positions().at(ib));
  Point c(mesh_.positions().at(ic));

  Vector ref = b - a;
  Vector target = pos - a;
  float det1 = ref.y * target.x - ref.x * target.y;

  ref = c - b;
  target = pos - b;
  float det2 = ref.y * target.x - ref.x * target.y;

  ref = a - c;
  target = pos - c;
  float det3 = ref.y * target.x - ref.x * target.y;

  if(det1 <= 0 && det2 <= 0 && det3 <= 0)
    return true;
  return false;
}

float GeneratedTerrain::getHeight(Point pos, int ia, int ib, int ic){
  Point a(mesh_.positions().at(ia));
  Point b(mesh_.positions().at(ib));
  Point c(mesh_.positions().at(ic));

  float distA = distance(pos, a);
  float distB = distance(pos, b);
  float distC = distance(pos, c);

  return (distA * a.z + distB * b.z + distC * c.z) / (distA + distB + distC);
}

Vector GeneratedTerrain::getNormal(Point pos, int ia, int ib, int ic){
  Point a(mesh_.positions().at(ia));
  Point b(mesh_.positions().at(ib));
  Point c(mesh_.positions().at(ic));

  Vector aV(mesh_.normals().at(ia));
  Vector bV(mesh_.normals().at(ib));
  Vector cV(mesh_.normals().at(ic));

  float distA = distance(pos, a);
  float distB = distance(pos, b);
  float distC = distance(pos, c);

  return normalize((distA * aV + distB * bV + distC * cV) / (distA + distB + distC));
}

void GeneratedTerrain::draw(const Transform& v, const Transform& p) {
  ::draw(mesh_, RotationX(90) * Scale(100,10,100), v, p) ;
}

void GeneratedTerrain::draw(const GLuint& shaders_program, Transform model, Transform view, Transform proj) {
  // configurer le pipeline 
  //glUseProgram(shaders_program);

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

  mesh_.draw(shaders_program);
}

void GeneratedTerrain::setCheckpoints(Transform transform) {
    unsigned int w = 1024;
    unsigned int h = 1024;

    // lecture des fichiers contenant les coordoonées des points
    Parser parser_points = Parser("MMachine/data/path_points.txt");
    Parser parser_points_radius = Parser("MMachine/data/path_points_radius.txt");

    assert(parser_points.getNbLines() == parser_points_radius.getNbLines());

    // création des checkpoints et ajout à la liste
    for(unsigned int i = 0;i < parser_points.getNbLines() - 1;i++) {
      Checkpoint c;
      c.center = Point(parser_points.get(i,0), parser_points.get(i,1), 0);
      c.radius_point = Point(parser_points_radius.get(i, 0), parser_points_radius.get(i,1), 0);
      checkpoints_.push_back(c);
    }
    // TODO radius doit être initialisé après que le point 2 (du rayon) ait été transformé

    for(Checkpoint &check : checkpoints_) {
      // normalisation et transformation du point au centre du checkpoint
      check.center.x = check.center.x / (w - 1);
      check.center.y = 1-(check.center.y / (h - 1));  // 1 - la valeur en y car les coordonnées données viennent d'un png, et les y sont inversés
      check.center = transform(check.center);

      // normalisation et transformation du point sur le rayon
      check.radius_point.x = check.radius_point.x / (w - 1);
      check.radius_point.y = 1-(check.radius_point.y / (h - 1));  // 1 - la valeur en y car les coordonnées données viennent d'un png, et les y sont inversés
      check.radius_point = transform(check.radius_point);

      // init du rayon
      check.radius = distance(check.center, check.radius_point);
    }
    // TODO convertir les vector en points
    Point  p1, p2;
    for(unsigned int i = 0;i < checkpoints_.size();i++) {
      p1 = checkpoints_[i].center;
      if(i == checkpoints_.size()-1) {
        p2 = checkpoints_[0].center;
      }else {
        p2 = checkpoints_[i+1].center;
      }
      
      Mesh mesh(GL_LINES);
      mesh.color(Red());
      mesh.vertex(p1.x, p1.y, p1.z);
      mesh.vertex(p2.x, p2.y, p2.z);
      meshs_checkpoints_.push_back(mesh);
      mesh.release();
    }
}

void GeneratedTerrain::drawCheckpoints(Transform model, Transform view, Transform proj) {
  for(Mesh m : meshs_checkpoints_) {
    ::draw(m, model, view, proj);
  }
}


std::vector<Checkpoint> GeneratedTerrain::getCheckpoints() const {
  return checkpoints_;
}

void GeneratedTerrain::release() {
  mesh_.release();
  for(Mesh m : meshs_checkpoints_) {
    m.release();
  }
}

//Flat Terrain

FlatTerrain::FlatTerrain(const Point& pmin, const Point& pmax) : mesh_(GL_TRIANGLES) {
  //terrain mesh
  unsigned int a = mesh_.vertex(pmin.x, pmin.y, 0.f) ;
  unsigned int b = mesh_.vertex(pmax.x, pmin.y, 0.f) ;
  unsigned int c = mesh_.vertex(pmax.x, pmax.y, 0.f) ;
  unsigned int d = mesh_.vertex(pmin.x, pmax.y, 0.f) ;

  mesh_.triangle(a, c, b) ;
  mesh_.triangle(a, d, c) ;
}

void FlatTerrain::project(const Point& from, Point& to, Vector& n) const {
  to.z = 0 ;
  n = Vector(0.f, 0.f, 1.f) ;
}

void FlatTerrain::draw(const Transform& v, const Transform& p) {
  ::draw(mesh_, Identity(), v, p) ;
}
