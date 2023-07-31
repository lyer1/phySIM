#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <cmath>
#include <iostream>
#include <random>

int HEIGHT       = 720,
    WIDTH        = 1280,
    NORMAL_FORCE = 10;

template <typename t> void print(t f){std::cout<<f<<std::endl;}
template <typename t> void print(t f, t g){std::cout<<f<<" "<<g<<std::endl;}
sf::Color colour_list[5] = {sf::Color::Red, sf::Color::Green, sf::Color::Blue, sf::Color::Magenta, sf::Color::Cyan};

void drawLineBetween(int a, int b, int c, int d, sf::RenderWindow &window){
    sf::Vertex line[] =
                        {
                        sf::Vertex(sf::Vector2f(a, b)),
                        sf::Vertex(sf::Vector2f(c, d))};

    window.draw(line, 2, sf::Lines);
};


struct RandomGenerator{
    std::random_device rd;
    std::mt19937 mt;
    RandomGenerator(){}

    int random_val(int size){
        return rd()%size;    
    }
};

class PhyObject: public sf::CircleShape{
    public:
        sf::Color colour;
		sf::Vector2f pos, prev_pos;
		int radius;
		float angle = 0, mass = 10;
        bool pinned = false;
		sf::Vector2f acc;

    PhyObject(sf::Color c = sf::Color::Green, sf::Vector2f p = sf::Vector2f(0, 0), int r = 3, bool pin = false): sf::CircleShape(r){
        colour = c;
        pos = p;
        prev_pos = p;
        radius = r;
        pinned = pin;
        acc = sf::Vector2f(0, 0);
        this->setFillColor(this->colour);
        this->setPosition(this->pos);
        // this->setOutlineColor(sf::Color::Black);
        // this->setOutlineThickness(3);
    }
    
    PhyObject(sf::Vector2f pos):PhyObject(sf::Color::Green, pos, 5){}
    PhyObject(sf::Vector2f p, int r):PhyObject(sf::Color::Blue, p, r){}
    PhyObject(sf::Vector2f p, int r, bool pin):PhyObject(sf::Color::Blue, p, r, pin){}

    void render(sf::RenderWindow &w){
        this->setPosition(this->pos);
        w.draw(*this);
    }

    void addVect(sf::Vector2f b){
        pos += b;
    }
    
    void updatePosition(float dt){
        if(pinned){
            acc = {};
            return;
        }
        sf::Vector2f disp = pos - prev_pos;

        prev_pos = pos;
        pos = pos + disp + acc * (dt*dt);
        
        acc = {};
    }

    void accelerate(sf::Vector2f a){
        acc += a;
    }
    void setVel(sf::Vector2f v, float dt){
        prev_pos = pos - (v * dt);
    }

    void addVel(sf::Vector2f v, float dt){
        prev_pos -= v * dt;
    }

    sf::Vector2f getVelocity(float dt){
        return (pos - prev_pos) / dt;
    }

    void applyForce(sf::Vector2f force){
        if(pinned){
            return;
        }
        sf::Vector2f accl = {force.x/mass, force.y/mass};
        this->accelerate(accl);
    
    }
};

class Stick{
    public:
        PhyObject *p1, *p2;
        bool spring = false;
        float  length;
        float spring_constant = 1000.0f;
    Stick() = default;
    Stick(PhyObject *p_1, PhyObject *p_2, float l, bool sp = false){
        p1 = p_1;
        p2 = p_2;
        length = l;
        spring = sp;

    }
    
    void reinforceStickConstraint(){
        sf::Vector2f dydx = p1->pos - p2->pos;
        float new_distance = sqrt(dydx.x * dydx.x + dydx.y * dydx.y);
        float percentage = ((length - new_distance) / new_distance )/ 2;
        
        dydx *= percentage;
        if(!spring){
            if(!p1->pinned)
                p1->pos += dydx;
            if(!p2->pinned)
                p2->pos -= dydx;
        }
        else{
            float dX = length - new_distance;
            sf::Vector2f spring_force = (dydx / new_distance) * -dX * spring_constant;
            if(!p1->pinned)
                p1->applyForce(spring_force);
            if(!p2->pinned)
                p2->applyForce(spring_force);
        }
    }
    void render(sf::RenderWindow &w){
        
        drawLineBetween(p1->pos.x + p1->radius, p1->pos.y + p1->radius, p2->pos.x + p2->radius, p2->pos.y + p2->radius, w);
    }
};

sf::Vector2f dotProd(sf::Vector2f v1, int v2){
        sf::Vector2f ret = {v1.x * v2, v1.y * v2};
        return ret;}

float distBwVec(sf::Vector2f a, sf::Vector2f b){
    float x = a.x - b.x;
    float y = a.y - b.y;
    
    return sqrt(x*x + y*y);
}
class PhyRec{
    public:
    PhyRec(sf::Vector2f loc, int p, int q, std::vector<PhyObject> &Object_arr, std::vector<Stick> &Stick_arr){
        PhyObject A(loc, 4), B(sf::Vector2f(loc.x + p, loc.y), 4), 
                  C(sf::Vector2f(loc.x, loc.y + q), 4), D(sf::Vector2f(loc.x + p, loc.y + q), 4);
        Object_arr.push_back(A); Object_arr.push_back(B); Object_arr.push_back(C); Object_arr.push_back(D);
        int l = Object_arr.size()-1;

        
        PhyObject *d = &Object_arr[l], *c = &Object_arr[l-1], *b = &Object_arr[l-2], *a = &Object_arr[l-3];
        Stick s0(a, b, p), s1(b, d, q), s2(d, c, p), s3(c, a, q), 
              diag(a, d, distBwVec(a->pos, d->pos)), diag2(b, c, distBwVec(b->pos, c->pos));
        
        Stick_arr.push_back(s0); Stick_arr.push_back(s1); Stick_arr.push_back(s2); Stick_arr.push_back(s3); 
        Stick_arr.push_back(diag); Stick_arr.push_back(diag2); 
    

    }
};


class Solver{
    public:
        std::vector<PhyObject> Object_arr;
        std::vector<Stick> Stick_arr;
    
        char                   sub_steps          = 1;
        sf::Vector2f           GRAVITY            = {0.0, 1500.0};
        float                  DRAG_COEFF         = 10.0f;
        float                  DAMPNING_COEFF     = 100.5f;
        float                  PI                 = 3.1415f;
        int                    size;
        int                    constraint_radius = 300;
        sf::Vector2f           constraint_center = {640, 300};
        struct RandomGenerator random_v;
        int                    substeps = 8; 

    
    Solver(){
        Object_arr.reserve(4000);
        Stick_arr.reserve(4000);
        int a, b;

        this->createCloth(50, 15, 15);
        for(int i = 0; i < 0; i++){    
            a = random_v.random_val(HEIGHT);
            b = random_v.random_val(WIDTH);
            // a = 100; b = 200;
            PhyObject new_obj_a = PhyObject(colour_list[random_v.random_val(5)], sf::Vector2<float>(b, a), 10 );
            
            a = random_v.random_val(HEIGHT);
            b = random_v.random_val(WIDTH);
            
            // a = 400; b = 200;

            PhyObject new_obj_b = PhyObject(colour_list[random_v.random_val(5)], sf::Vector2<float>(b, a), 10, true);
            Object_arr.push_back(std::move(new_obj_a));
            Object_arr.push_back(std::move(new_obj_b));
            
            // Stick stick = Stick(&new_obj_a, &new_obj_b, distBwVec(new_obj_a.pos, new_obj_b.pos));
            Stick stick;
            int l = Object_arr.size();
            stick.p1 = &Object_arr[l-1];
            stick.p2 = &Object_arr[l-2];
            stick.length = distBwVec(new_obj_a.pos, new_obj_b.pos);
            
            Stick_arr.push_back(std::move(stick));
        }    
    };

    Solver(int size){
        this->size = size;
        float a = 640, b = 200;
        for(int i = 0; i < size; i++){
        PhyObject & new_obj = *(new PhyObject(colour_list[random_v.random_val(5)], sf::Vector2<float>(b, a), 2 + random_v.random_val(10) ));
        a = random_v.random_val(HEIGHT);
        b = random_v.random_val(WIDTH);        
        Object_arr.emplace_back(new_obj);
        }
    }

    Solver(int stick_number, bool stick_mode){
        int a, b;
        Object_arr.reserve(100);
        Stick_arr.reserve(50);
        for(int i = 0; i < stick_number; i++){
            a = random_v.random_val(HEIGHT);
            b = random_v.random_val(WIDTH);
            
            PhyObject *new_obj_a = new PhyObject(colour_list[random_v.random_val(5)], sf::Vector2<float>(b, a), 7 );
            
            a = random_v.random_val(HEIGHT);
            b = random_v.random_val(WIDTH);
            PhyObject *new_obj_b = new PhyObject(colour_list[random_v.random_val(5)], sf::Vector2<float>(b, a), 7 );
            
            Stick* stick = new Stick(new_obj_a, new_obj_b, distBwVec(new_obj_a->pos, new_obj_b->pos));

            Stick_arr.push_back( std::move(*stick));
            Object_arr.push_back( std::move(*new_obj_a));
            Object_arr.push_back( std::move(*new_obj_b));
            // print(Stick_arr[0].p1);
            // print(new_obj_a);
            // print(Object_arr[0]);
            
        }   
    }
    void applyGravity(){
        for (auto& obj : Object_arr) {
            obj.accelerate(GRAVITY);
        }
    }
    void updateObjects(float dt){
        for (auto& obj : Object_arr) {
            obj.updatePosition(dt);
        }
    }
    void collisonGravityRenderLoop(float dt, sf::RenderWindow &window){

        const float    response_coef = 0.1f;
        const uint64_t objects_count = Object_arr.size();
        
        
        
        for (uint64_t i{0}; i < objects_count; ++i) {
            PhyObject& object_1 = Object_arr[i];
            
            for(int steps = 0; steps < sub_steps; steps++){
            for (uint64_t k{i + 1}; k < objects_count; ++k) {

                PhyObject& object_2 = Object_arr[k];
                
                const sf::Vector2f collison_axis = object_1.pos - object_2.pos;

                const float collison_axis_dist_sq = collison_axis.x * collison_axis.x + collison_axis.y * collison_axis.y;
                
                const float min_dist = object_1.radius + object_2.radius;
                
                // drawLineBetween(object_2.pos.x, object_2.pos.y, object_1.pos.x, object_1.pos.y, window);
                // Check overlapping
                if (collison_axis_dist_sq < min_dist * min_dist) {
                    float collison_axis_dist = sqrt(collison_axis_dist_sq);

                    const sf::Vector2f n = collison_axis / collison_axis_dist;

                    const float mass_ratio_1 = object_1.mass / (object_1.mass + object_2.mass);
                    const float mass_ratio_2 = object_2.mass / (object_1.mass + object_2.mass);
                    const float delta        = response_coef * (min_dist - collison_axis_dist);


                    object_1.pos += n * (mass_ratio_2 * delta)  ;
                    object_2.pos -= n * (mass_ratio_1 * delta);

                    
                    }
            }
            sf::Vector2f vel = object_1.prev_pos - object_1.pos;
            float virtual_radius = object_1.radius <= 1? 5 : object_1.radius;
            sf::Vector2f drag_force = 0.5f * DRAG_COEFF * vel * PI * virtual_radius * virtual_radius;

            //update stuff on object

            object_1.accelerate(GRAVITY);
            object_1.applyForce(drag_force);

            object_1.updatePosition(dt);

            applyConstraintWindow(object_1);
            }
            object_1.render(window);
            
            
        }
    }

    void stickRender(sf::RenderWindow &window){
        for(auto f: Stick_arr){
            for(int steps = 0; steps < sub_steps; steps++){
                f.reinforceStickConstraint();}
            
            f.render(window);
        }
    }

    void applyConstraintCirc(PhyObject &obj){
        
        const sf::Vector2f v = constraint_center - obj.pos; //disp vector
        const float dist = sqrt(v.x * v.x + v.y * v.y);
        if (dist > (constraint_radius - obj.radius)) {
            sf::Vector2f n = v / dist; // unit vector {direction}
            float f = (constraint_radius - obj.radius);
            obj.pos = constraint_center - n * f;
        }
    
    }

    void applyConstraintWindow(PhyObject &obj){
        const float disp_y = obj.pos.y + obj.radius;
        if((disp_y > HEIGHT)|(disp_y < 0)){
            sf::Vector2f vel = obj.pos - obj.prev_pos;
            if(disp_y > HEIGHT)
                obj.pos.y = HEIGHT - obj.radius;
            if(disp_y < 0)
                obj.pos.y = obj.radius;

            obj.prev_pos.y = obj.pos.y + vel.y - (vel.y / DAMPNING_COEFF);
            
        } 
        const float disp_x = obj.pos.x - obj.radius; 
        if((disp_x > WIDTH)|(disp_x <= 0)){
            sf::Vector2f vel = obj.pos - obj.prev_pos;
            if(disp_x > WIDTH)
                obj.pos.x = WIDTH - obj.radius;
            if(disp_x <= 0)
                obj.pos.x = obj.radius;
            
            obj.prev_pos.x = obj.pos.x + vel.x - (vel.x / DAMPNING_COEFF);
        }
         
    
    }
    void createCloth(int p, int q, int diff = 20){
        int idx, S_idx = Stick_arr.size();
        int st = 260;
        int a, b = 100;
        std::vector<std::vector<PhyObject>> point_mass_mat;

        for(int i = 0; i < q; i++){
            std::vector<PhyObject> temp = {};
            a = st;
            for(int j = 0; j < p; j ++){
                temp.emplace_back(PhyObject(sf::Vector2f(a, b), 1));
                
                a += diff;
            }
            
            point_mass_mat.push_back(temp);
            b += diff;
        }
        PhyObject *ptr1, *ptr2;
        //p = 6         q = 2
        for(int i = 0; i < q; i++){
            // Object_arr.push_back(point_mass_mat[i][0]);
            Object_arr.push_back(point_mass_mat[i][0]);
            idx = Object_arr.size() - 1;
            ptr1 = &Object_arr[idx];
            if(i==0){
                ptr1->pinned = true;
            }
            else{
                ptr2 = &Object_arr[idx - p];
                Stick stick_current_vertical(ptr1, ptr2, diff);
                Stick_arr.push_back(std::move(stick_current_vertical));
            }
            for(int j = 1; j < p; j ++){
                //0 1**push, 1 2**push
                
                Object_arr.push_back(point_mass_mat[i][j]);

                idx++;
                ptr1 = &Object_arr[idx];
                ptr2 = &Object_arr[idx-1];

                Stick stick_current_horizontal(ptr1, ptr2, diff);
                // std::cout<<idx-1<<" "<<idx<<std::endl;
                // ptr1 = ptr2;
                Stick_arr.push_back(std::move(stick_current_horizontal));
                if(i == 0){
                    ptr1->pinned = true;
                    continue;}
                ptr2 = &Object_arr[idx - p];
                Stick stick_current_vertical(ptr2, ptr1, diff);
                Stick_arr.push_back(std::move(stick_current_vertical));

                

            }
            
        }
        

    }
};