#include "player.hpp"

#include <SDL2/SDL.h>

Player::Player() : 
    terrain_(nullptr),
    forward_(true),
    switchable_(true),
    controller_(nullptr),
    active_(false),
    speed_(0.f, 0.f, 0.f),
    last_time_(0),
    position_(0.f, 0.f, 0.f),
    direction_(1.f, 0.f, 0.f),
    normal_(0.f, 0.f, 0.f),
        //useful parameters to control the movement behavior
    acceleration_(0.0005f),
    turning_angle_(3.f),
    max_speed_(8.f),
    friction_(0.02f, 0.1f), //front and lateral friction for drift
    fallen_(false)
{}

void Player::spawn_at(const Point& position, const Vector& direction) {
  //reset position on terrain
  position_ = position ;
  direction_ = direction ;
  project(position_) ;

  //reset speed
  speed_ = Vector(0.f, 0.f, 0.f) ;

  //wait for activation
  deactivate() ;
}

void Player::step() {
  //do not move if inactive
  if(!active_) return ;

  //handle friction
  Vector linear = dot(speed_, direction_)*direction_ ;
  Vector lateral = speed_ - linear ;
  speed_ = speed_ - friction_.x * linear ;
  speed_ = speed_ - friction_.y * lateral ;

  //backward or forward
  if(forward_) {
    //handle accelerations
    if(controller_->up()) {
      speed_ = speed_ + direction_ * acceleration_ ;
    }

    if(controller_->down()) {
      speed_ = speed_ - direction_ * acceleration_ ;
      if(dot(speed_, direction_) < 0) {
        speed_ = Vector(0.f, 0.f, 0.f) ;
      }
    }

    //handle rotation
    if(controller_->left()) {
      direction_ = Rotation(normal_, turning_angle_)(direction_) ;
    }
    if(controller_->right()) {
      direction_ = Rotation(normal_, -turning_angle_)(direction_) ;
    }
  } else {
    //handle accelerations
    if(controller_->up()) {
      speed_ = speed_ + direction_ * acceleration_ ;
      if(dot(speed_, direction_) > 0) {
        speed_ = Vector(0.f, 0.f, 0.f) ;
      }
    }

    if(controller_->down()) {
      speed_ = speed_ - direction_ * acceleration_ ;
    }

    //handle rotation
    if(controller_->left()) {
      direction_ = Rotation(normal_, -turning_angle_)(direction_) ;
    }
    if(controller_->right()) {
      direction_ = Rotation(normal_, turning_angle_)(direction_) ;
    }
  }

  //handle the switch breaking // moving backwards
  //control needs to be released then pushed again
  if(length(speed_) == 0.f) {
    if(switchable_) {
      if(controller_->down()) {
        forward_ = false ;
      } 
      if(controller_->up()) {
        forward_ = true ;
      }
    }
  }

  switchable_ = !(
    controller_->up() 
    || controller_->down() 
    || controller_->left() 
    || controller_->right()
    ) ;

  //compute new position position
  int time = SDL_GetTicks() ;
  Point new_position = position_ + (time - last_time_) * speed_ ;

  //project
  project(new_position);

  //update speed taking projection into account
  speed_ = (new_position - position_) / (time - last_time_) ;
  speed_ = speed_ - dot(speed_, normal_)*normal_ ;

  //update the position
  position_ = new_position ;

  //update time
  last_time_ = time ;
}

bool Player :: active() {
  return active_ ;
}

void Player::activate() {
  active_ = true ;
}

void Player::deactivate() {
  active_ = false ;
}

Transform Player::transform() {
  return Transform(direction_, cross(normal_, direction_), normal_, position_ - Point()) ;
}

void Player::set_terrain(const Terrain* terrain) {
  terrain_ = terrain ;
}

void Player::set_controller(const Controller* controller) {
  controller_ = controller ;
}

void Player::set_bounding_box(const Point p1, const Point p2) {
  // Définition de la taille de chaque véhicule sur chaque axe
  sizeX_ = fabs(p2.x - p1.x) / 2.0;
  sizeY_ = fabs(p2.y - p1.y) / 2.0;
  sizeZ_ = fabs(p2.z - p1.z) / 2.0;
}

float Player::get_x() {
  return position_.x;
}

float Player::get_y() {
  return position_.y;
}

float Player::get_z() {
  return position_.z;
}

void Player::project(Point& candidate) {
  Vector newNormal = normal_;

  // projette le joueur sur le terrain
  ((GeneratedTerrain*)terrain_)->project(position_, candidate, newNormal, this);
  // gère les collisions
  collide(position_, normal_, candidate, newNormal);

  normal_ = newNormal;

  // recalcule la direction
  direction_ = normalize(direction_ - dot(direction_, normal_)*normal_) ;
}

bool Player::collide(const Point& oldPos, const Vector& oldNorm, Point& newPos, Vector& newNorm) {
  // collision avec le terrain
  bool collTerrain = collideWithTerrain(oldPos, oldNorm, newPos, newNorm);
  // collision avec l'autre joueur
  bool collPlayer = collideWithPlayer(oldPos, oldNorm, newPos, newNorm);

  // collision avec les obstacles
  bool collObjects = objectsManager_->collideWithPlayer(oldPos, oldNorm, newPos, newNorm);

  return collPlayer && collTerrain && collObjects;
}

bool Player::collideWithTerrain(const Point& oldPos, const Vector& oldNorm, Point& newPos, Vector& newNorm){  
  float angleNormZ = dot(newNorm, Vector(0.f, 0.f, 1.f));
  // Vérification de la raideur de la pente
  if(angleNormZ < 0.75){
    // Autorise seulement de descendre une pente trop raide, mais pas de la monter
    Vector dir = normalize(direction_ - dot(direction_, newNorm)*newNorm);
    float angleDirZ = dot(dir, Vector(0.f, 0.f, 1.f)); 
    if(angleDirZ > 0){
      newPos = oldPos;
      newNorm = oldNorm;
    }
    return true;
  }
  return false;
}

bool Player::collideWithPlayer(const Point& oldPos, const Vector& oldNorm, Point& newPos, Vector& newNorm){
  // vecteur entre le centre du joueur adverse et la position du joueur courant
  Vector p = playerToCollide_->getPosition() - position_;
  // récupération des points de la bounding box adverse
  std::vector<Point> corners = playerToCollide_->getCornerPoints();
  // pour chaque point de la bounding box, on vérifie s'il est dans la bounding box du joueur courant.
  for (std::vector<Point>::iterator it = corners.begin(); it!= corners.end(); ++it){
    Vector pc = p + Vector(*it);
    if(pc.x >= -sizeX_ && pc.x <= sizeX_ &&
        pc.y >= -sizeY_ && pc.y <= sizeY_ &&
        pc.z >= -sizeZ_ && pc.z <= sizeZ_){

      Vector knockback = (-(pc + p) / 2.0 ) * 0.1; // collision -> on applique un knockback au joueur courant

      newPos = newPos + knockback; // maj avec knockback
      return true;
    }
  }
  
  return false;
}

Vector Player::getDirection(){
  return direction_;
}

Point Player::getPosition(){
  return position_;
}

void Player::setOtherPlayer(Player& player){
  playerToCollide_ = &player;
}

void Player::setObjectsManager(ObjectsManager* objectsManager) {
  objectsManager_ = objectsManager;
}

std::vector<Point> Player::getCornerPoints(){
  std::vector<Point> corners;
  corners.push_back(Point(sizeX_, sizeY_, sizeZ_));
  corners.push_back(Point(sizeX_, sizeY_, -sizeZ_));
  corners.push_back(Point(sizeX_, -sizeY_, sizeZ_));
  corners.push_back(Point(sizeX_, -sizeY_, -sizeZ_));
  corners.push_back(Point(-sizeX_, sizeY_, sizeZ_));
  corners.push_back(Point(-sizeX_, sizeY_, -sizeZ_));
  corners.push_back(Point(-sizeX_, -sizeY_, sizeZ_));
  corners.push_back(Point(-sizeX_, -sizeY_, -sizeZ_));
  return corners;
}

bool Player::isFallen(){
  return fallen_;
}

void Player::setCurrentFallingDist(float currentFallingDist){
  // distance de glissade à 0 : pas de chute -> reset
  if(currentFallingDist <= 0.0){
    fallingDist_ = 0.0;
    fallen_ = false;
  }
  // glissade -> on incrémente la distance de glissade totale
  else{
    fallingDist_ += currentFallingDist;
    if(fallingDist_ >= 2.0) // Trop de glissade = chute! 
      fallen_ = true;
  }
}