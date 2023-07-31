#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <SFML/Graphics/Color.hpp>


#include <cmath>
#include <iostream>
#include <random>

int HEIGHT       = 500,
    WIDTH        = 500,
    NORMAL_FORCE = 10;

template <typename t> void print(t f){std::cout<<f<<std::endl;}
template <typename t> void print(t f, t g){std::cout<<f<<" "<<g<<std::endl;}
sf::Color colour_list[5] = {sf::Color::Red, sf::Color::Green, sf::Color::Blue, sf::Color::Magenta, sf::Color::Cyan};


struct RandomGenerator{
    std::random_device rd;
    std::mt19937 mt;
    RandomGenerator(){}

    int random_val(int size){
        return rd()%size;    
    }
};


struct PixelObject{
        bool empty = true;
        sf::Color colour = sf::Color::Red;
        int id = 0;
    PixelObject() = default;
    PixelObject(sf::Color c){
        colour = c;
    }
};

class Solver{
    public:
        PixelObject **main_arr;
        int W, H;
        RandomGenerator rand;
    Solver(int W, int H){
        this->W = W;
        this->H = H;
        main_arr = new PixelObject*[W];
        for(int i = 0; i < H; i++){
            main_arr[i] = new PixelObject[W];
        }
    }
    void update(){
        // int W = , H = 500;
        PixelObject temp;
        int counter = 0;
        for(int i = W-1 ; i > 0; i--){
            for(int j = H - 1; j > 0; j--){
                sf::Vector2i prev(i, j);
                temp = main_arr[i][j];
                counter++;
                if(temp.empty)
                    continue;
                if((temp.empty!=true)&&(temp.id == 1))
                    continue;
                if(main_arr[i][j+1].empty){
                    main_arr[i][j].empty = true;
                    // main_arr[i][j].colour = sf::Color::Black;
                    main_arr[i][j+1].empty = false;
                    main_arr[i][j+1].id = 0;
                    main_arr[i][j].id = 0;
                    main_arr[i][j+1].colour = temp.colour;
                }
                else if((main_arr[i+1][j].empty)&&(counter%2)){
                    // std::cout<<i%2<<std::endl;
                    main_arr[i][j].empty = true;
                    main_arr[i+1][j].id = 0;
                    main_arr[i][j].id = 0;
                    // main_arr[i][j].colour = sf::Color::Black;
                    main_arr[i+1][j].empty = false;
                    main_arr[i+1][j].colour = temp.colour;
                }
                else if((main_arr[i-1][j].empty)){
                    // std::cout<<"aa";

                    main_arr[i][j].empty = true;
                    // main_arr[i][j].colour = sf::Color::Black;
                    main_arr[i-1][j].empty = false;
                    main_arr[i-1][j].id = 0;
                    main_arr[i][j].id = 0;
                    main_arr[i-1][j].colour = temp.colour;
                }
                if( ((i >= 0)&&(i <= W)) ||((j<=H)&&(j>=0)) )
                    continue;
                    
            }
        }
    }
};

class SandRec{
    public:
        sf::Vector2i pos;
        int h, b;
    SandRec(sf::Vector2i r_pos, int a, int c){
        pos = r_pos;
        h = a;
        b = c;
    }
};

void transpose_to_image(sf::Image &image, Solver &s, int H, int W){
    for(int i = 0; i < W; i++){
            for(int j = 0; j < H; j++){
                if(!s.main_arr[i][j].empty)
                    image.setPixel(i, j, s.main_arr[i][j].colour);
                else 
                    image.setPixel(i, j, sf::Color::Black);
            }
        }
}

int main(){
    const unsigned int W = 800;
    const unsigned int H = 800; // you can change this to full window size later
    const unsigned int steps = 4;

    sf::RenderWindow window(sf::VideoMode(W, H), "Test", sf::Style::Titlebar | sf::Style::Resize | sf::Style::Close);
    sf::View view(sf::FloatRect(W, H, W, H));

    
    // window.setView(view);
    // view.zoom(0.5);
    sf::Event event; sf::Color temp_col;
    sf::Image image;
    sf::Mouse mouse; sf::Vector2i ms;

    Solver s(W, H);

    image.create(W, H, sf::Color::Black);
    // image.loadFromFile()
    window.setFramerateLimit(60);
    sf::Texture texture;
    texture.loadFromImage(image);
    sf::Sprite sprite;
    sprite.setTexture(texture, true);

    int check_matrix_x[12] = {0, 0, 0, 0, 1, -1, 1, -1, 2, -2, 2, -2};
    int check_matrix_y[12] = {0, 0, 0, 0, -1, 1, -1, 1, -2, 2, -2, 2};

    while (window.isOpen()){
        window.clear();
        for(int f = 0; f < steps; f++)
            s.update();
        transpose_to_image(image, s, W, H);
        texture.loadFromImage(image);
        sprite.setTexture(texture, true);
        window.draw(sprite);
        window.display();

        ms = mouse.getPosition(window);
        
        

        if(mouse.isButtonPressed(sf::Mouse::Button::Left)){
                    
            sf::Vector2f pos1, pos2;
            s.main_arr[ms.x][ms.y].empty = false;
            s.main_arr[ms.x][ms.y].id = 0;
            s.main_arr[ms.x][ms.y].colour = sf::Color::Red;
            
            
            for(auto i: check_matrix_x){
                for(auto j: check_matrix_y){
                    s.main_arr[ms.x + i][ms.y + j].empty = false;
                    s.main_arr[ms.x + i][ms.y + j].id = 0;
                    s.main_arr[ms.x + i][ms.y + j].colour = sf::Color::Red;
                }
            }         

        }
        if(mouse.isButtonPressed(sf::Mouse::Button::Right)){
                    
            sf::Vector2f pos1, pos2;
            s.main_arr[ms.x][ms.y].empty = false;
            s.main_arr[ms.x][ms.y].id = 1;
            s.main_arr[ms.x][ms.y].colour = sf::Color::White;
            
            
            for(auto i: check_matrix_x){
                for(auto j: check_matrix_y){
                    s.main_arr[ms.x + i][ms.y + j].empty = false;
                    s.main_arr[ms.x + i][ms.y + j].id = 1;  
                    s.main_arr[ms.x + i][ms.y + j].colour = sf::Color::White;
                }
            }
                    

        }
        
        while (window.pollEvent(event))
        {
                    
            if (event.type == sf::Event::Closed){
                window.close();
            }
            
            if(event.type == sf::Event::MouseButtonPressed){
                

            }
        }
    }
}