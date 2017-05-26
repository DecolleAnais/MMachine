#include "src/terrain.hpp"
#include "src/controller.hpp"
#include "src/player.hpp"

#include "mat.h"
#include "wavefront.h"
#include "orbiter.h"
#include "draw.h"
#include "app.h"
#include "texture.h"

#include "program.h"
#include "uniforms.h"

#include <chrono>
#include <cmath>

typedef std::chrono::high_resolution_clock Clock;

class Play : public App
{
public:
    // constructeur : donner les dimensions de l'image, et eventuellement la version d'openGL.
    Play( ) : 
      App(1024, 640), 
      controller1_(SDLK_UP, SDLK_DOWN, SDLK_LEFT, SDLK_RIGHT),
      controller2_('z', 's', 'q', 'd'),
      terrain_(Point(-20.f, -20.f, 0.f), Point(20.f, 20.f, 0.f))
    {}
    
    int init( )
    {
        // Init des meshs des véhicules
        vehicule1_ = read_mesh("MMachine/mmachine.obj") ;
        vehicule1_.default_color(Color(1.0f, 0.f, 0.f)) ;
        vehicule2_ = read_mesh("MMachine/mmachine.obj") ;
        vehicule2_.default_color(Color(0.0f, 0.f, 1.f)) ;

        // Init des joueurs
        Point bound_p1, bound_p2;
        joueur1_.set_terrain(&terrain_) ;
        joueur1_.set_controller(&controller1_) ;
        joueur2_.set_terrain(&terrain_) ;
        joueur2_.set_controller(&controller2_) ;
        
        joueur1_.setOtherPlayer(joueur2_);
        joueur2_.setOtherPlayer(joueur1_);

        joueur1_.spawn_at(Point(89.0,27.0,0), Vector(1,0,0), bound_p1, bound_p2) ;
        joueur1_.activate() ;
        joueur2_.spawn_at(Point(89.0,25.0,0), Vector(1,0,0), bound_p1, bound_p2) ;
        joueur2_.activate() ;

        oldPmin_ = Point(std::min(joueur1_.get_x(), joueur2_.get_x()),
                    std::min(joueur1_.get_y(), joueur2_.get_y()),
                    std::max(joueur1_.get_z(), joueur2_.get_z()));
        oldPmax_ = Point(std::max(joueur1_.get_x(), joueur2_.get_x()),
                    std::max(joueur1_.get_y(), joueur2_.get_y()),
                    std::max(joueur1_.get_z(), joueur2_.get_z()));

        // Init des boites englobantes
        Point p1, p2;
        vehicule1_.bounds(p1, p2);
        joueur1_.set_bounding_box(p1, p2);
        vehicule2_.bounds(p1, p2);
        joueur2_.set_bounding_box(p1, p2);

        // Init des textures
        textures.resize(2);
        samplers.resize(2);

        textures[0] = read_texture(0, "MMachine/textures/grass.png");
        glGenSamplers(1, &samplers[0]);
        glSamplerParameteri(samplers[0], GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glSamplerParameteri(samplers[0], GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glSamplerParameteri(samplers[0], GL_TEXTURE_WRAP_S, GL_REPEAT);
        glSamplerParameteri(samplers[0], GL_TEXTURE_WRAP_T, GL_REPEAT);

        textures[1] = read_texture(1, "MMachine/textures/route.png");
        glGenSamplers(1, &samplers[1]);
        glSamplerParameteri(samplers[1], GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glSamplerParameteri(samplers[1], GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glSamplerParameteri(samplers[1], GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
        glSamplerParameteri(samplers[1], GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

        textures[2] = read_texture(2, "MMachine/textures/dirt.png");
        glGenSamplers(1, &samplers[2]);
        glSamplerParameteri(samplers[2], GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glSamplerParameteri(samplers[2], GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glSamplerParameteri(samplers[2], GL_TEXTURE_WRAP_S, GL_REPEAT);
        glSamplerParameteri(samplers[2], GL_TEXTURE_WRAP_T, GL_REPEAT);

        // Init du shader
        m_program = read_program("MMachine/vertex_fragment_shaders.glsl");
        program_print_errors(m_program);

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
        release_program(m_program);
        vehicule1_.release();
        vehicule2_.release();
        glDeleteSamplers(1, &samplers[2]);
        glDeleteSamplers(1, &samplers[1]);
        glDeleteSamplers(1, &samplers[0]);
        glDeleteTextures(1, &textures[2]);   
        glDeleteTextures(1, &textures[1]);   
        glDeleteTextures(1, &textures[0]);   
        return 0;
    }
    
    Transform updateCamera(){
        // centre la caméra
        Point pmin(std::min(joueur1_.get_x(), joueur2_.get_x()),
                    std::min(joueur1_.get_y(), joueur2_.get_y()),
                    std::max(joueur1_.get_z(), joueur2_.get_z()));
        Point pmax(std::max(joueur1_.get_x(), joueur2_.get_x()),
                    std::max(joueur1_.get_y(), joueur2_.get_y()),
                    std::max(joueur1_.get_z(), joueur2_.get_z()));

        Clock::time_point time = Clock::now();
        float delta = (float)std::chrono::duration_cast<std::chrono::milliseconds>(time - oldTime_).count() / 1000.0;
        oldTime_ = time;

        // détection d'un trop grand champ de caméra, recentre la caméra sur le joueur 1
        float coeffSpeed = 7.5;
        float maxDistPlayers = 27.0f;
        Point pminT, pmaxT;
        if(length(pmax - pmin) >= maxDistPlayers) {
            pminT = Point(joueur1_.get_x(), joueur1_.get_y(), joueur1_.get_z());
            pmaxT = Point(joueur1_.get_x(), joueur1_.get_y(), joueur1_.get_z());
        }
        else{
            pminT = pmin;
            pmaxT = pmax;
        }
        
        float pminXS = (pminT.x - oldPmin_.x) * coeffSpeed;
        float pminYS = (pminT.y - oldPmin_.y) * coeffSpeed;
        float pmaxXS = (pmaxT.x - oldPmax_.x) * coeffSpeed;
        float pmaxYS = (pmaxT.y - oldPmax_.y) * coeffSpeed;
        
        pmin.x = oldPmin_.x + pminXS * delta;
        pmin.y = oldPmin_.y + pminYS * delta;
        pmax.x = oldPmax_.x + pmaxXS * delta;
        pmax.y = oldPmax_.y + pmaxYS * delta;

        float dist = distance(pmin, pmax);
        float cameraDist = (-1.0 * powf(dist, 2.0) / maxDistPlayers) + (2.0 * dist) + 15.0;
        //std::cout << dist << " " << cameraDist << std::endl;
        Point cameraPos = center(pmin, pmax) + Vector(0, 0, std::max(0.0f, cameraDist));
        //std::cout << cameraPos << std::endl;

        oldPmin_ = pmin;
        oldPmax_ = pmax;

        return Lookat(cameraPos, center(pmin, pmax), Vector(0, 1, 0));
    }

    // dessiner une nouvelle image
    int render( )
    {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // deplace la camera
        // int mx, my;
        // unsigned int mb= SDL_GetRelativeMouseState(&mx, &my);
        // if(mb & SDL_BUTTON(1))              // le bouton gauche est enfonce
        //     m_camera.rotation(mx, my);
        // else if(mb & SDL_BUTTON(3))         // le bouton droit est enfonce
        //     m_camera.move(mx);
        // else if(mb & SDL_BUTTON(2))         // le bouton du milieu est enfonce
        //     m_camera.translation((float) mx / (float) window_width(), (float) my / (float) window_height());


        // déplace les joueurs
        Transform player1_pos = joueur1_.transform() ;
        Transform player2_pos = joueur2_.transform() ;

        // déplace la caméra & récupère la projection
        Transform view = updateCamera();
        Transform projection = Perspective(90, (float) window_width() / (float) window_height(), 0.1f, 100.0f);

        /***** AFFICHAGE *****/
        // VEHICULES
        draw(vehicule1_, player1_pos, view, projection) ;
        draw(vehicule2_, player2_pos, view, projection) ;

        // TERRAIN
        glUseProgram(m_program);
        // textures
        program_use_texture(m_program, "texture0", 0, textures[0], samplers[0]);
        program_use_texture(m_program, "texture1", 1, textures[1], samplers[1]);
        program_use_texture(m_program, "texture2", 2, textures[2], samplers[2]);

        // phares
        program_uniform(m_program, "spotP1Pos", joueur1_.getPosition());  
        program_uniform(m_program, "spotP2Pos", joueur2_.getPosition());
        program_uniform(m_program, "spotP1Dir", joueur1_.getDirection());
        program_uniform(m_program, "spotP2Dir", joueur2_.getDirection());

        // affichage
        terrain_.draw(m_program, Identity(), view, projection);

        /***** CONTROLES *****/
        //reset
        if(key_state('r')) {
            Point bound_p1, bound_p2;
            vehicule1_.bounds(bound_p1, bound_p2);   // points de la boite englobante du mesh

            joueur1_.spawn_at(Point(0,0,0), Vector(0,1,0), bound_p1, bound_p2) ;
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

    GeneratedTerrain terrain_;
    std::vector<GLuint> textures;
    std::vector<GLuint> samplers;

    Point oldPmin_;
    Point oldPmax_;
    std::chrono::high_resolution_clock::time_point oldTime_;

    float camera_x_max;
    float camera_y_max;

    GLuint m_program;
};


int main( int argc, char **argv )
{
    //GeneratedTerrain terrain = GeneratedTerrain();

    Play app;
    app.run();
    
    return 0;
}
