#include <GLFW/glfw3.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include <stdbool.h>

#define PI 3.14159265358979323846
#define MAXY 400.0f
#define MAXX 400.0f
#define G 0.1
#define FARZ 800.0f
#define NEARZ 1.0f
#define DELTA_T (1000.0f/60.0f)
#define STEP_SIZE 5 
#define TRAIL_LENGTH 50

typedef struct {
    double m;
    float r,x,y,z,vx,vy,vz;
    int sectors,stacks;
    float color[3];
    float trail[3][TRAIL_LENGTH];
    int trail_idx;
}Planet;

void apply_gravity(Planet *p, Planet *o) {
    float dx=o->x-p->x;
    float dy=o->y-p->y;
    float dz=o->z-p->z;
    
    float distSq=dx*dx+dy*dy+dz*dz;
    float D=sqrt(distSq);

    if (D<0.1f) return; 

    float forceMag=(G*o->m)/(distSq*D);

    p->vx+=forceMag*dx;
    p->vy+=forceMag*dy;
    p->vz+=forceMag*dz;
}

void move_planet(Planet *p){
    p->x+=p->vx*DELTA_T;
    p->y+=p->vy*DELTA_T;
    p->z+=p->vz*DELTA_T;

    p->trail[0][p->trail_idx]=p->x;
    p->trail[1][p->trail_idx]=p->y;
    p->trail[2][p->trail_idx]=p->z;
    p->trail_idx=(p->trail_idx+1)%TRAIL_LENGTH;
}

void draw_Planet(Planet *p){
    glPushMatrix();
    
    glColor3f(p->color[0],p->color[1],p->color[2]);

    for(int i=0;i<p->stacks;++i){
        float alfa_0=PI*(-0.5f+(float)i/p->stacks);
        float z0=p->r*sin(alfa_0);
        float rz0=p->r*cos(alfa_0);

        float alfa_1=PI*(-0.5f+(float)(i+1)/p->stacks);
        float z1=p->r*sin(alfa_1);
        float rz1=p->r*cos(alfa_1);

        glBegin(GL_QUAD_STRIP);

        for(int j=0;j<=p->sectors;++j){
            float beta=2.0f*PI*(float)j/p->sectors;
            float x0=rz0*cos(beta),x1=rz1*cos(beta);
            float y0=rz0*sin(beta),y1=rz1*sin(beta);

            glNormal3f(x0/p->r,y0/p->r,z0/p->r);
            glVertex3d(p->x+x0,p->y+y0,p->z+z0);

            glNormal3f(x1/p->r,y1/p->r,z1/p->r);
            glVertex3d(p->x+x1,p->y+y1,p->z+z1);
        }

        glEnd();
    }

    glPopMatrix();
}

void draw_grid(int size,float step){
    glDisable(GL_LIGHTING);
    glBegin(GL_LINES);
    glColor3f(0.5,0.5,0.5);

    for(float i=-size;i<=size;i+=step){
        glVertex3f(i,-size,0.0f);
        glVertex3f(i,size,0.0f);

        glVertex3f(-size,i,0.0f);
        glVertex3f(size,i,0.0f);
    }

    glEnd();
    glEnable(GL_LIGHTING);
}

void draw_trail(Planet *p){
    glDisable(GL_LIGHTING);
    glBegin(GL_LINE_STRIP);
    
    for(int i=0;i<TRAIL_LENGTH;++i){
        int idx=(p->trail_idx+i)%TRAIL_LENGTH;
        
        float alpha=(float)i/TRAIL_LENGTH;
        glColor4f(p->color[0],p->color[1],p->color[2],alpha);

        glVertex3f(p->trail[0][idx],p->trail[1][idx],p->trail[2][idx]);
    }

    glEnd();
    glEnable(GL_LIGHTING);
}

int main(){
    glfwInit();
    GLFWwindow* window = glfwCreateWindow(2*MAXX,2*MAXY,"Window",NULL,NULL);
    glfwMakeContextCurrent(window);

    glEnable(GL_DEPTH_TEST);

    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    float light_source_pos[]={400.0f,600.0f,400.0f,1.0f};
    glLightfv(GL_LIGHT0,GL_POSITION,light_source_pos);

    glEnable(GL_COLOR_MATERIAL);
    glColorMaterial(GL_FRONT,GL_AMBIENT_AND_DIFFUSE);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glFrustum(-1.0,1.0,-1.0,1.0,NEARZ,FARZ);
    glMatrixMode(GL_MODELVIEW);
    
    int n=5;
    Planet **planets=malloc(n*sizeof(Planet*));
    bool *inactive=malloc(n*sizeof(bool));
    memset(inactive,false,n*sizeof(bool));

    for(int i=0;i<n;++i){
        planets[i]=malloc(sizeof(Planet));
        planets[i]->trail_idx=0;

        for(int j=0;j<TRAIL_LENGTH;++j){
            planets[i]->trail[0][j]=0.0f;
            planets[i]->trail[1][j]=0.0f;
            planets[i]->trail[2][j]=0.0f;
        }
    }

    // -------------------------------------------------
    *planets[0] = (Planet){1000.0, 14.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 32, 32, {1.0f, 0.9f, 0.2f}, {{0}}, 0};

    *planets[1] = (Planet){2, 3.5f, 45.0f, 0.0f, 0.0f, 0.0f, 0.365f, 0.0f, 20, 20, {0.2f, 0.5f, 1.0f}, {{0}}, 0};

    *planets[2] = (Planet){3, 4.5f, 75.0f, 0.0f, 0.0f, 0.0f, 0.282f, 0.0f, 22, 22, {0.9f, 0.4f, 0.2f}, {{0}}, 0};

    *planets[3] = (Planet){5, 3.8f, 105.0f, 0.0f, 10.0f, 0.0f, 0.239f, 0.089f, 20, 20, {0.8f, 0.8f, 0.8f}, {{0}}, 0};

    *planets[4] = (Planet){20, 3.2f, 140.0f, 0.0f, 0.0f, -30.0f, 0.207f, 0.0f, 20, 20, {0.9f, 0.7f, 0.2f}, {{0}}, 0};

    // inactive[0]=true;
    // -------------------------------------------------

    float angle=0.0f,step=0.5f; 

    while (!glfwWindowShouldClose(window)) {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        glLoadIdentity();
        
        glTranslatef(0.0f,0.0f,-200.0f);
        
        glRotatef(-30.0f,1.0f,0.0f,0.0f);
        glRotatef(45.0f,0.0f,0.0f,1.0f);

        // glRotatef(angle,0.0,1.0,1.0);
        // angle+=step;
        
        draw_grid(MAXX,STEP_SIZE); 

        for(int i=0;i<n;++i){
            for(int j=0;j<n;++j){
                if(j==i) continue;
                apply_gravity(planets[i],planets[j]);
            }
        }
        
        for(int i=0;i<n;++i) if(!inactive[i]) move_planet(planets[i]);

        for(int i=0;i<n;++i){
            draw_Planet(planets[i]);
            if(!inactive[i]) draw_trail(planets[i]);
        }

        glfwSwapBuffers(window);
        glfwPollEvents();

        Sleep(DELTA_T);
    }

    glfwTerminate();

    for(int i=0;i<n;++i) free(planets[i]);
    free(planets);
    free(inactive);

    return 0;
}