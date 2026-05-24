#include <GLFW/glfw3.h>
#include <math.h>
#include <stdio.h>
#include <windows.h>
#include <stdbool.h>
#include <stdlib.h>
#include <omp.h>
#include <time.h>

#define PI 3.14159265358979323846
#define MAX_NODE_NUM 400000
#define MAX_BODY_NUM 10000
#define MAX_RECURSION_DEPTH 30
#define THETA 0.5
#define G 0.016
#define DELTA 0.01
#define EPS 3.0
#define POINT_SIZE 5.0 
#define COLLISION_RADIUS 5.5
#define STARTING_SPREAD 700.0
#define MAXX 800
#define MAXY 800
#define INITIAL_VEL 1000
#define NUM_THREADS 12

// quad tree
typedef struct Body{
    double x,y,mass,vx,vy;
    float color[3];
}Body;

typedef struct Node{
    double x1,y1,x2,y2,mass,cx,cy,side;
    int body_idx;
    int children[4];
}Node;

Body bodies[MAX_BODY_NUM];

int new_node_pos=0;
Node quad_tree[MAX_NODE_NUM];

int create_node(double x1,double y1,double x2,double y2){
    if(new_node_pos>=MAX_NODE_NUM) return -1;
    quad_tree[new_node_pos].x1=x1;
    quad_tree[new_node_pos].y1=y1;
    quad_tree[new_node_pos].x2=x2;
    quad_tree[new_node_pos].y2=y2;
    quad_tree[new_node_pos].side=max(x2-x1,y2-y1);
    quad_tree[new_node_pos].body_idx=-1;
    quad_tree[new_node_pos].mass=0.0;
    quad_tree[new_node_pos].cx=0.0;
    quad_tree[new_node_pos].cy=0.0;
    for(int i=0;i<4;++i) quad_tree[new_node_pos].children[i]=-1;
    return new_node_pos++;
}

void reset_tree(){
    new_node_pos=0;
}

bool is_leaf(int root){
    return quad_tree[root].children[0]==-1;    
}

enum QUAD_NUM{
    BOTTOM_LEFT,
    BOTTOM_RIGHT,
    TOP_LEFT,
    TOP_RIGHT
};

int get_quad_number(int root,int body_idx){
    double xm=(quad_tree[root].x1+quad_tree[root].x2)/2.0;
    double ym=(quad_tree[root].y1+quad_tree[root].y2)/2.0;

    if(bodies[body_idx].y<=ym){
        if(bodies[body_idx].x<=xm) return BOTTOM_LEFT;
        else return BOTTOM_RIGHT;
    }

    if(bodies[body_idx].x<=xm) return TOP_LEFT;
    else return TOP_RIGHT;
}

void split_node(int root){
    double xm=(quad_tree[root].x1+quad_tree[root].x2)/2.0;
    double ym=(quad_tree[root].y1+quad_tree[root].y2)/2.0;
    quad_tree[root].children[0]=create_node(quad_tree[root].x1,quad_tree[root].y1,xm,ym);
    quad_tree[root].children[1]=create_node(xm,quad_tree[root].y1,quad_tree[root].x2,ym);
    quad_tree[root].children[2]=create_node(quad_tree[root].x1,ym,xm,quad_tree[root].y2);
    quad_tree[root].children[3]=create_node(xm,ym,quad_tree[root].x2,quad_tree[root].y2);
}

void agregate_vals(int root,int body_idx){
    quad_tree[root].cx+=bodies[body_idx].x*bodies[body_idx].mass;
    quad_tree[root].cy+=bodies[body_idx].y*bodies[body_idx].mass;
    quad_tree[root].mass+=bodies[body_idx].mass;
}

void calc_avg_vals(int root){
    if(root==-1) return;
    
    if(quad_tree[root].mass>0.0){
        quad_tree[root].cx/=quad_tree[root].mass;
        quad_tree[root].cy/=quad_tree[root].mass;
    }

    for(int i=0;i<4;++i) calc_avg_vals(quad_tree[root].children[i]);
}

void insert_body(int root,int body_idx,int depth){
    if(root==-1) return;

    if(depth>MAX_RECURSION_DEPTH){
        quad_tree[root].body_idx=body_idx;
        return;
    }

    agregate_vals(root,body_idx);

    if(is_leaf(root) && quad_tree[root].body_idx==-1){
        quad_tree[root].body_idx=body_idx;
        return;
    }

    if(is_leaf(root)){
        int old_body_idx=quad_tree[root].body_idx;
        quad_tree[root].body_idx=-1;

        split_node(root);
        
        int quad_old=get_quad_number(root,old_body_idx);
        insert_body(quad_tree[root].children[quad_old],old_body_idx,depth+1);
        
        int quad_new=get_quad_number(root,body_idx);
        insert_body(quad_tree[root].children[quad_new],body_idx,depth+1);

        return;
    }

    int quad=get_quad_number(root,body_idx);
    insert_body(quad_tree[root].children[quad],body_idx,depth+1);
}

void apply_force(int root,int body_idx,double *fx,double *fy){
    if(root==-1 || quad_tree[root].mass==0.0) return;
    
    if(is_leaf(root) && quad_tree[root].body_idx==body_idx) return;

    double dx=quad_tree[root].cx-bodies[body_idx].x;
    double dy=quad_tree[root].cy-bodies[body_idx].y;
    double d=sqrt(dx*dx+dy*dy+EPS);
    
    if(d==0.0) return;

    if(is_leaf(root) || quad_tree[root].side/d<=THETA){
        double F=G*quad_tree[root].mass*bodies[body_idx].mass/(d*d);
        *fx+=F*(dx/d);
        *fy+=F*(dy/d);
        return;
    }

    for(int i=0;i<4;++i){
        if(quad_tree[root].children[i]!=-1){
            apply_force(quad_tree[root].children[i],body_idx,fx,fy);
        }
    }
}

void insert_bodies(){
    for(int i=0;i<MAX_BODY_NUM;++i) insert_body(0,i,0);
    calc_avg_vals(0);
}

void update_positions(){
    double fx=0.0,fy=0.0;

    #pragma omp parallel for num_threads(NUM_THREADS) private(fx,fy)
    for(int i=0;i<MAX_BODY_NUM;++i){
        fx=0.0,fy=0.0;
        apply_force(0,i,&fx,&fy);
        
        bodies[i].vx+=(fx/bodies[i].mass)*DELTA;
        bodies[i].x+=bodies[i].vx*DELTA;
        
        bodies[i].vy+=(fy/bodies[i].mass)*DELTA;
        bodies[i].y+=bodies[i].vy*DELTA;
    }
}

// collision detection
bool in_bounds(int root,int body_idx){
    return quad_tree[root].x1<=bodies[body_idx].x+COLLISION_RADIUS && 
            quad_tree[root].x2>=bodies[body_idx].x-COLLISION_RADIUS && 
            quad_tree[root].y1<=bodies[body_idx].y+COLLISION_RADIUS &&
            quad_tree[root].y2>=bodies[body_idx].y-COLLISION_RADIUS;
}

void collide_bodies(int body_a,int body_b){
    double dx=bodies[body_a].x-bodies[body_b].x;
    double dy=bodies[body_a].y-bodies[body_b].y;
    double d=sqrt(dx*dx+dy*dy+EPS);

    if(d>2.0*COLLISION_RADIUS) return;
    
    double nx=dx/d;
    double ny=dy/d;

    double dvx=bodies[body_a].vx-bodies[body_b].vx;
    double dvy=bodies[body_a].vy-bodies[body_b].vy;
    double norm_vel=dvx*nx+dvy*ny;

    if(norm_vel>0) return;

    double e=1.0; 
    
    double ma=bodies[body_a].mass;
    double mb=bodies[body_b].mass;

    double j=-(1.0+e)*norm_vel;
    j/=(1.0/ma+1.0/mb);

    double impulse_x=j*nx;
    double impulse_y=j*ny;

    bodies[body_a].vx+=impulse_x/ma;
    bodies[body_a].vy+=impulse_y/ma;
    
    bodies[body_b].vx-=impulse_x/mb;
    bodies[body_b].vy-=impulse_y/mb;

    double overlap=(2.0*COLLISION_RADIUS)-d;

    double slop=0.05; 
    double percent=0.5; 

    if (overlap>slop){
        double correction=(overlap/(1.0/ma+1.0/mb))*percent;
        
        double cx=correction*nx;
        double cy=correction*ny;
        
        bodies[body_a].x+=cx/ma;
        bodies[body_a].y+=cy/ma;
 
        bodies[body_b].x-=cx/mb;
        bodies[body_b].y-=cy/mb;
    }
}

void apply_collisions(int root,int body_idx){
    if(quad_tree[root].body_idx==body_idx) return;

    if(is_leaf(root) && quad_tree[root].body_idx!=-1){
        collide_bodies(quad_tree[root].body_idx,body_idx);
        return;
    }

    for(int i=0;i<4;++i){
        if(quad_tree[root].children[i]!=-1 && in_bounds(quad_tree[root].children[i],body_idx)){
            apply_collisions(quad_tree[root].children[i],body_idx);
        }
    }
}

void check_collisions(){
    for(int i=0;i<MAX_BODY_NUM;++i) apply_collisions(0,i);
}

// simulation
void get_bounds(double *mxx,double *mnx,double *mxy,double *mny){
    *mxx=bodies[0].x,*mnx=bodies[0].x;
    *mxy=bodies[0].y,*mny=bodies[0].y;
    for(int i=0;i<MAX_BODY_NUM;++i){
        *mxx=max(*mxx,bodies[i].x);
        *mnx=min(*mnx,bodies[i].x);
        *mxy=max(*mxy,bodies[i].y);
        *mny=min(*mny,bodies[i].y);
    }
}

void init_first_node(){
    double mxx,mnx,mxy,mny;
    get_bounds(&mxx,&mnx,&mxy,&mny);
    double dx=mxx-mnx;
    double dy=mxy-mny;
    double d=max(dx,dy)+EPS;
    double cx=mnx+dx/2,cy=mny+dy/2;
    create_node(cx-d/2,cy-d/2,cx+d/2,cy+d/2);
}

// sorting with Z shapes 
unsigned int spread_bits(unsigned int x){
    x&= 0x000003ff;
    x=(x|(x<<16))&0x030000ff;
    x=(x|(x<<8))&0x0300f00f;
    x=(x|(x<<4))&0x030c30c3;
    x=(x|(x<<2))&0x09249249;
    return x;
}

unsigned int get_morton_code(double x,double y){
    unsigned int ix=(unsigned int)((x+MAXX)/2*MAXX*1023.0);
    unsigned int iy=(unsigned int)((y+MAXY)/2*MAXY*1023.0);
    
    return (spread_bits(iy)<<1)|spread_bits(ix);
}

int compare_bodies(const void* a, const void* b){
    Body* b1=(Body*)a;
    Body* b2=(Body*)b;
    
    unsigned int mc1=get_morton_code(b1->x,b1->y);
    unsigned int mc2=get_morton_code(b2->x,b2->y);
    
    return (mc1>mc2)-(mc1<mc2);
}

void simulate_gravity(){
    reset_tree();
    init_first_node();
    insert_bodies();
    qsort(bodies,MAX_BODY_NUM,sizeof(Body),compare_bodies);
    update_positions();
    check_collisions();
}

void render_body(int body_idx){
    glPointSize(POINT_SIZE);

    glEnable(GL_POINT_SMOOTH);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glBegin(GL_POINTS);
    glColor3f(bodies[body_idx].color[0],bodies[body_idx].color[1],bodies[body_idx].color[2]);

    glVertex2d(bodies[body_idx].x,bodies[body_idx].y);

    glEnd();
}

void draw(){
    for(int i=0;i<MAX_BODY_NUM;++i) render_body(i);
}

double randf(double min, double max){
    return min+(max-min)*((double)rand()/RAND_MAX);
}

void init_bodies(){
    double cx=0.0;
    double cy=0.0;

    for(int i=0;i<MAX_BODY_NUM;++i){

        double angle=randf(0.0,2.0*PI);

        double r=sqrt(randf(0.0,1.0))*STARTING_SPREAD;

        double x=cx+cos(angle)*r;
        double y=cy+sin(angle)*r;

        double mass=randf(150.0,500.0);

        double tx=-sin(angle);
        double ty=cos(angle);

        double speed=INITIAL_VEL/sqrt(r+10.0);

        bodies[i].x=x;
        bodies[i].y=y;

        bodies[i].vx=tx*speed;
        bodies[i].vy=ty*speed;

        bodies[i].mass=mass;

        bodies[i].color[0]=(float)randf(0.0,1.0);
        bodies[i].color[1]=(float)randf(0.0,1.0);
        bodies[i].color[2]=(float)randf(0.0,1.0);
    }

    bodies[0].x=cx;
    bodies[0].y=cy;

    bodies[0].vx=0.0;
    bodies[0].vy=0.0;

    bodies[0].mass=10000000.0;

    bodies[0].color[0]=0.0;
    bodies[0].color[1]=0.0;
    bodies[0].color[2]=0.0;
}

int main(){
    glfwInit();
    GLFWwindow* window = glfwCreateWindow(2*MAXX,2*MAXY,"Window",NULL,NULL);
    glfwMakeContextCurrent(window);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(-MAXX, MAXX,-MAXY,MAXY,-1,1);
    glMatrixMode(GL_MODELVIEW);

    init_bodies();

    struct timespec start,end;
    int frames=0;
    char title[256];

    clock_gettime(CLOCK_MONOTONIC,&start);

    while(!glfwWindowShouldClose(window)){
        glClear(GL_COLOR_BUFFER_BIT);

        simulate_gravity();
        draw();

        glfwSwapBuffers(window);
        glfwPollEvents();

        ++frames;
        clock_gettime(CLOCK_MONOTONIC,&end);
        double spent=end.tv_sec-start.tv_sec+(end.tv_nsec-start.tv_nsec)/1e9;

        if(spent>=1.0){
            sprintf(title,"FPS:%d",frames);
            glfwSetWindowTitle(window,title);
            frames=0;
            start=end;
        }
    }
    
    glfwTerminate();
    return 0;
}