#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <SFML/System.hpp>
#include "defs.hpp"

#include <iostream>

//constants


float delta_t = 0.05;


int main(){
    //window init

    sf::RenderWindow window(sf::VideoMode(WIDTH, HEIGHT), "Test", sf::Style::Titlebar | sf::Style::Resize | sf::Style::Close);
    sf::Event event;
    sf::Mouse mouse; sf::Vector2f mouse_arr;
    sf::Keyboard keyboard;

    
    window.setFramerateLimit(60);
    
    
    Solver solver;
    sf::Vector2f horizontal_force = {100.0f, 0.0f};
    //loop
    int wait = 0;
    sf::Vector2i ms;
    PhyObject *pinched_obj = NULL;

    while (window.isOpen()){
        window.clear();
        
        if(wait){
            wait--;
            continue;
        }
        ms = mouse.getPosition(window);
        mouse_arr = {(float)mouse.getPosition(window).x, (float)mouse.getPosition(window).y};
                
        
        if(mouse.isButtonPressed(sf::Mouse::Button::Right)){
            sf::Vector2f pos1, pos2;
            int f = 5;
            ms = mouse.getPosition(window);
            for(int i=0; i < solver.Stick_arr.size(); i++){
                Stick curr_obj = solver.Stick_arr[i];
                pos1 = curr_obj.p1->pos + sf::Vector2f(-f, -f);
                pos2 = curr_obj.p2->pos + sf::Vector2f(f, f);
                
                if(  ((pos1.x <= ms.x) && (ms.x <= pos2.x)) && ((pos1.y <= ms.y) && (ms.y <= pos2.y))){
                    solver.Stick_arr.erase(solver.Stick_arr.begin() + i);
                    break;
                }
            }
        }

        if(mouse.isButtonPressed(sf::Mouse::Button::Left)&&(pinched_obj))
            pinched_obj->pos = mouse_arr;
        else
            pinched_obj = NULL;
        
        // drawLineBetween(100, 100, ms.x, ms.y, window);
        solver.collisonGravityRenderLoop(0.02, window);

        solver.stickRender(window);
        window.display();


        while (window.pollEvent(event))
        {
                    
            if (event.type == sf::Event::Closed){
                window.close();
            }

            if (event.type == sf::Event::KeyPressed){
                if(keyboard.isKeyPressed(sf::Keyboard::LControl)){
                    PhyRec(sf::Vector2f(ms.x, ms.y), 30, 30, solver.Object_arr, solver.Stick_arr);
                }
            }
            
            
            if(event.type == sf::Event::MouseButtonPressed){
                if(mouse.isButtonPressed(sf::Mouse::Button::Left)){
                    
                    sf::Vector2f pos1, pos2;
                    int f = 5;
                    ms = mouse.getPosition(window);
                    for(int i=0; i < solver.Object_arr.size(); i++){
                        PhyObject curr_obj = solver.Object_arr[i];
                        pos1 = curr_obj.pos + sf::Vector2f(-f, -f);
                        pos2 = curr_obj.pos + sf::Vector2f(f, f);
                        
                        if(  ((pos1.x <= ms.x) && (ms.x <= pos2.x)) && ((pos1.y <= ms.y) && (ms.y <= pos2.y))){
                            pinched_obj = &solver.Object_arr[i];
                            
                            break;
                            }
                        }
                    }
                else
                    pinched_obj = NULL;
                
                    
            }
            
        
        }
        
    }



    return 0;
}