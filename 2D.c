#include <GLFW/glfw3.h>
#include <math.h>
#include <stdio.h>
#include <windows.h>
#include <stdbool.h>


#define G 6.67430
#define PI 3.14159265358979323846
#define MAXY 400.0f
#define MAXX 400.0f
#define DELTA_T (1000.0f/60.0f)
#define SMOO 100

typedef struct{
    float x,y,r,vx,vy,mass;
    float color[3];
}Circle;

void apply_gravity(Circle *a,Circle *b){
    float dx=b->x-a->x;
    float dy=b->y-a->y;
    
    float distSq=dx*dx+dy*dy+0.1f;
    float dist=sqrt(distSq);
    float distCubed=distSq*dist;

    float common=(G*b->mass)/distCubed;
    
    a->vx+=dx*common;
    a->vy+=dy*common;
}

void move_circle(Circle *c) {
    c->x+=c->vx;
    c->y+=c->vy;
}

void draw_circle(Circle *c,int num_segments){
    glColor3f(c->color[0],c->color[1],c->color[2]);

    glBegin(GL_TRIANGLE_FAN);

    glVertex2f(c->x,c->y);

    for(int i=0;i<=num_segments;++i){
        float arg=2*PI*i/num_segments;
        float x=c->x+c->r*cos(arg);
        float y=c->y+c->r*sin(arg);
        glVertex2d(x,y);
    }

    glEnd();
}

int main(){
    glfwInit();
    GLFWwindow* window = glfwCreateWindow(2*MAXX,2*MAXY,"Window",NULL,NULL);
    glfwMakeContextCurrent(window);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(-MAXX, MAXX,-MAXY,MAXY,-1,1);
    glMatrixMode(GL_MODELVIEW);

    Circle c1;c1.r=25;c1.x=0;c1.y=0;c1.vx=0;c1.vy=0;c1.mass=3000.0f;c1.color[0]=1.0f;c1.color[1]=0.8f;c1.color[2]=0.0f;

    Circle c;c.r=8;c.x=180;c.y=0;c.vx=0;c.vy=5.2f;c.mass=25.0f;c.color[0]=1.0f;c.color[1]=1.0f;c.color[2]=0.0f;

    Circle c2;c2.r=12;c2.x=280;c2.y=0;c2.vx=0;c2.vy=4.2f;c2.mass=10.0f;c2.color[0]=0.6f;c2.color[1]=0.2f;c2.color[2]=0.8f;

    Circle c3;c3.r=12;c3.x=400;c3.y=0;c3.vx=0;c3.vy=4.2f;c3.mass=50.0f;c3.color[0]=0.0f;c3.color[1]=0.2f;c3.color[2]=0.8f;


    while (!glfwWindowShouldClose(window)) {
        glClear(GL_COLOR_BUFFER_BIT);

        apply_gravity(&c,&c1);
        apply_gravity(&c,&c2);
        apply_gravity(&c,&c3);

        apply_gravity(&c1,&c);
        apply_gravity(&c1,&c2);
        apply_gravity(&c1,&c3);
        
        apply_gravity(&c2,&c);
        apply_gravity(&c2,&c1);
        apply_gravity(&c2,&c3);

        apply_gravity(&c3,&c);
        apply_gravity(&c3,&c1);
        apply_gravity(&c3,&c2);

        move_circle(&c);
        move_circle(&c1);
        move_circle(&c2);
        move_circle(&c3);

        draw_circle(&c1,SMOO);
        draw_circle(&c,SMOO);
        draw_circle(&c2,SMOO);
        draw_circle(&c3,SMOO);

        glfwSwapBuffers(window);
        glfwPollEvents();

        Sleep(DELTA_T);
    }

    glfwTerminate();
    return 0;
}