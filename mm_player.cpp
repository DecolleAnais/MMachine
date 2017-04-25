#include "src/terrain.hpp"
#include "src/controller.hpp"
#include "src/player.hpp"

#include "mat.h"
#include "wavefront.h"
#include "orbiter.h"
#include "draw.h"
#include "app.h"

#include <chrono>

typedef std::chrono::high_resolution_clock Clock;

class Play : public App
{
public:
    // constructeur : donner les dimensions de l'image, et eventuellement la version d'openGL.
    Play( ) : 
      App(1024, 640), 
      controller1_(SDLK_UP, SDLK_DOWN, SDLK_LEFT, SDLK_RIGHT),
      controller2_('z', 's', 'q', 'd'),
      terrain_(Point(-20.f, -20.f, 0.f), Point(20.f, 20.f, 0.f)),
      generatedTerrain_()
    {}
    
    int init( )
    {
        vehicule1_ = read_mesh("MMachine/mmachine.obj") ;
        vehicule1_.default_color(Color(1.0f, 0.f, 0.f)) ;
        vehicule2_ = read_mesh("MMachine/mmachine.obj") ;
        vehicule2_.default_color(Color(0.0f, 0.f, 1.f)) ;

        //joueur1_.set_terrain(&terrain_) ;
        joueur1_.set_terrain(&generatedTerrain_) ;
        joueur1_.set_controller(&controller1_) ;
        joueur1_.spawn_at(Point(0,0,0), Vector(0,1,0)) ;
        joueur1_.activate() ;

        //joueur2_.set_terrain(&terrain_) ;
        joueur2_.set_terrain(&generatedTerrain_) ;
        joueur2_.set_controller(&controller2_) ;
        joueur2_.spawn_at(Point(1,1,0), Vector(0,1,0)) ;
        joueur2_.activate() ;

        oldPmin_ = Point(-20.f, -20.f, -20.f);
        oldPmax_ = Point(20.f, 20.f, 20.f);
        m_camera.lookat(oldPmin_, oldPmax_);


        // etat openGL par defaut
        glClearColor(0.2f, 0.2f, 0.2f, 1.f);        // couleur par defaut de la fenetre
        
        glClearDepth(1.f);                          // profondeur par defaut
        glDepthFunc(GL_LESS);                       // ztest, conserver l'intersection la plus proche de la camera
        glEnable(GL_DEPTH_TEST);                    // activer le ztest
        
        return 0;   // ras, pas d'erreur
    }
    
    // destruction des objets de l'application
    int quit( )
    {
        vehicule1_.release();
        vehicule2_.release();
        return 0;
    }
    
    // dessiner une nouvelle image
    int render( )
    {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // deplace la camera
        int mx, my;
        unsigned int mb= SDL_GetRelativeMouseState(&mx, &my);
        if(mb & SDL_BUTTON(1))              // le bouton gauche est enfonce
            m_camera.rotation(mx, my);
        else if(mb & SDL_BUTTON(3))         // le bouton droit est enfonce
            m_camera.move(mx);
        else if(mb & SDL_BUTTON(2))         // le bouton du milieu est enfonce
            m_camera.translation((float) mx / (float) window_width(), (float) my / (float) window_height());


        // déplace les joueurs
        Transform player1_pos = joueur1_.transform() ;
        Transform player2_pos = joueur2_.transform() ;


        // centre la caméra
        Point pmin(std::min(joueur1_.get_x(), joueur2_.get_x()),
                    std::min(joueur1_.get_y(), joueur2_.get_y()),
                    -10);
        Point pmax(std::max(joueur1_.get_x(), joueur2_.get_x()),
                    std::max(joueur1_.get_y(), joueur2_.get_y()),
                    10);
        m_camera.lookat(pmin, pmax);

        // détection d'un trop grand champ de caméra, recentre la caméra sur le joueur 1
        float coeffSpeed = 10.0;
        if(length(pmax - pmin) >= 27) {
            Point pminT = Point(joueur1_.get_x()-1, joueur1_.get_y()-1, -10);
            Point pmaxT = Point(joueur1_.get_x()+1, joueur1_.get_y()+1, 10);
            float pminXS = (pminT.x - oldPmin_.x) * coeffSpeed;
            float pminYS = (pminT.y - oldPmin_.y) * coeffSpeed;
            float pmaxXS = (pmaxT.x - oldPmax_.x) * coeffSpeed;
            float pmaxYS = (pmaxT.y - oldPmax_.y) * coeffSpeed;

            Clock::time_point time = Clock::now();
            float delta = (float)std::chrono::duration_cast<std::chrono::milliseconds>(time - oldTime_).count() / 1000.0;
            oldTime_ = time;
            pmin.x = oldPmin_.x + pminXS * delta;
            pmin.y = oldPmin_.y + pminYS * delta;
            pmax.x = oldPmax_.x + pmaxXS * delta;
            pmax.y = oldPmax_.y + pmaxYS * delta;

            m_camera.lookat(pmin, pmax);
        }
        else{
            oldTime_ = Clock::now();
        }

        oldPmin_ = pmin;
        oldPmax_ = pmax;

        // dessine les véhicules et le terrain
        draw(vehicule1_, player1_pos, m_camera) ;
        draw(vehicule2_, player2_pos, m_camera) ;

        //terrain_.draw(m_camera.view(), m_camera.projection(window_width(), window_height(), 45.f)) ;
        generatedTerrain_.draw(m_camera.view(), m_camera.projection(window_width(), window_height(), 45.f)) ;

        //reset
        if(key_state('r')) {
          joueur1_.spawn_at(Point(0,0,0), Vector(0,1,0)) ;
          joueur1_.activate() ;
        }

        return 1;
    }

protected:
    Mesh vehicule1_;
    Mesh vehicule2_;
    Player joueur1_;
    Player joueur2_;
    KeyboardController controller1_ ;
    KeyboardController controller2_ ;

    FlatTerrain terrain_ ;
    GeneratedTerrain generatedTerrain_;

    Orbiter m_camera;
    Point oldPmin_;
    Point oldPmax_;
    std::chrono::high_resolution_clock::time_point oldTime_;

    float camera_x_max;
    float camera_y_max;
};


int main( int argc, char **argv )
{
    //GeneratedTerrain terrain = GeneratedTerrain();

    Play app;
    app.run();
    
    return 0;
}
