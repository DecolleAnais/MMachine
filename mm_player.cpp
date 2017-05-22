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

        // textures
        textures[0] = read_texture(0, "data/papillon.png");
        textures[1] = read_texture(0, "data/debug2x2red.png");
        textures[2] = read_texture(0, "data/pacman.png");

        // chargement shader
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
        glDeleteTextures(1, &textures[2]);   
        glDeleteTextures(1, &textures[1]);   
        glDeleteTextures(1, &textures[0]);   
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

        int plage = 20;
        Point pmin(std::min(joueur1_.get_x(), joueur2_.get_x())-plage,
                    std::min(joueur1_.get_y(), joueur2_.get_y()+plage),
                    -plage);
        Point pmax(std::max(joueur1_.get_x(), joueur2_.get_x()-plage),
                    std::max(joueur1_.get_y(), joueur2_.get_y()+plage),
                    plage);
        m_camera.lookat(pmin, pmax);

        Clock::time_point time = Clock::now();
        float delta = (float)std::chrono::duration_cast<std::chrono::milliseconds>(time - oldTime_).count() / 1000.0;
        oldTime_ = time;

        // détection d'un trop grand champ de caméra, recentre la caméra sur le joueur 1
        float coeffSpeed = 5.0;
        Point pminT, pmaxT;
        if(length(pmax - pmin) >= 27) {
            pminT = Point(joueur1_.get_x()-plage, joueur1_.get_y()-plage, -plage);
            pmaxT = Point(joueur1_.get_x()+plage, joueur1_.get_y()+plage, plage);
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

        m_camera.lookat(pmin, pmax);

        oldPmin_ = pmin;
        oldPmax_ = pmax;

        // dessine les véhicules et le terrain
        draw(vehicule1_, player1_pos, m_camera) ;
        draw(vehicule2_, player2_pos, m_camera) ;

        // dessiner avec le shader program
        // configurer le pipeline 
        glUseProgram(m_program);

        // configurer le shader program
        // . recuperer les transformations
        Transform model = RotationX(90) * Scale(1,1,1); //RotationX(global_time() / 20);
        Transform view = m_camera.view();
        Transform projection = m_camera.projection(window_width(), window_height(), 45);
        
        // . composer les transformations : model, view et projection
        Transform mv = view * model;
        Transform mvp = projection * view * model;


        // . parametrer le shader program :
        //   . transformation : la matrice declaree dans le vertex shader s'appelle mvpMatrix

        program_uniform(m_program, "mvMatrix", mv);        
        program_uniform(m_program, "normalMatrix", mv.normal());        
        program_uniform(m_program, "mvpMatrix", mvp);
        program_uniform(m_program, "modelMatrix", model);
        program_uniform(m_program, "viewMatrix", view);
        program_uniform(m_program, "viewInvMatrix", view.inverse());

        program_use_texture(m_program, "texture0", 0, textures[0]);
        program_use_texture(m_program, "texture1", 0, textures[1]);
        program_use_texture(m_program, "texture2", 0, textures[2]);


        //terrain_.draw(m_camera.view(), m_camera.projection(window_width(), window_height(), 45.f)) ;
        //generatedTerrain_.draw(m_camera.view(), m_camera.projection(window_width(), window_height(), 45.f)) ;
        generatedTerrain_.draw(m_program);

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
    GLuint* textures;

    Orbiter m_camera;
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
