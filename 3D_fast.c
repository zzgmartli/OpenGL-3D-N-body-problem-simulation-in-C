#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include <stdbool.h>

#define MAX_BODY_NUM 1000
#define MAX_NODES 8*MAX_BODY_NUM

typedef struct{
    double x,y,z;
}Vec3;

typedef struct{
    Vec3 p1,p2,c;
    double side,mass;
    int body_idx;
    int children[8];
}Node;

typedef struct{
    Vec3 p,v,col;
    double mass;
}Body;

int cnt=0;
Node qt[MAX_NODES];
Body bods[MAX_BODY_NUM];

enum CUBE_NUM{
    BOTTOM_LEFT_BACK,
    BOTTOM_RIGHT_BACK,
    TOP_LEFT_BACK,
    TOP_RIGHT_BACK,
    BOTTOM_LEFT_FRONT,
    BOTTOM_RIGHT_FRONT,
    TOP_LEFT_FRONT,
    TOP_RIGHT_FRONT
};

int init_node(Vec3 p1,Vec3 p2){
    int curr=++cnt;
    qt[curr].p1=p1;
    qt[curr].p2=p2;
    qt[curr].c=(Vec3){0.0f,0.0f,0.0f};
    qt[curr].side=max(max(p1.x-p2.x,p1.y-p2.y),p1.z-p2.z);
    qt[curr].mass=0.0f;
    qt[curr].body_idx=-1;
    for(int i=0;i<8;++i) qt[curr].children[i]=-1;
    return curr;
}

bool is_leaf(int curr){
    return qt[curr].children[0]==-1;
}

int get_cube_num(int curr,int bod_id){
    double xm=(qt[curr].p1.x+qt[curr].p2.x)/2.0f,x=bods[bod_id].p.x;
    double ym=(qt[curr].p1.y+qt[curr].p2.y)/2.0f,y=bods[bod_id].p.y;
    double zm=(qt[curr].p1.z+qt[curr].p2.z)/2.0f,z=bods[bod_id].p.z;

    if(x<xm){
        if(z<zm){
            if(y<ym) return BOTTOM_LEFT_BACK; 
            else return BOTTOM_RIGHT_BACK;
        }else{
            if(y<ym) return TOP_LEFT_BACK; 
            else return TOP_RIGHT_BACK;
        }
    }else{
        if(z<zm){
            if(y<ym) return BOTTOM_LEFT_FRONT; 
            else return BOTTOM_RIGHT_FRONT;
        }else{
            if(y<ym) return TOP_LEFT_FRONT; 
            else return TOP_RIGHT_FRONT;
        }
    }

    return -1;
}

void split_node(int curr){
    double xm=qt[curr].p1.x-qt[curr].p2.x;
    double ym=qt[curr].p1.y-qt[curr].p2.y;
    double zm=qt[curr].p1.z-qt[curr].p2.z;

    qt[curr].children[0]=init_node(qt[curr].p1,(Vec3){xm,ym,zm});
    qt[curr].children[1]=init_node((Vec3){qt[curr].p1.x,ym,qt[curr].p1.z},(Vec3){xm,qt[curr].p2.y,zm});
    qt[curr].children[2]=init_node((Vec3){qt[curr].p1.x,qt[curr].p1.y,zm},(Vec3){xm,ym,qt[curr].p2.z});
    qt[curr].children[3]=init_node((Vec3){qt[curr].p1.x,ym,zm},(Vec3){xm,qt[curr].p2.y,qt[curr].p2.z});
    qt[curr].children[4]=init_node((Vec3){xm,qt[curr].p1.y,qt[curr].p1.z},(Vec3){qt[curr].p2.x,ym,zm});
    qt[curr].children[5]=init_node((Vec3){xm,ym,qt[curr].p1.z},(Vec3){qt[curr].p2.x,qt[curr].p2.y,zm});
    qt[curr].children[6]=init_node((Vec3){xm,qt[curr].p1.y,zm},(Vec3){qt[curr].p2.x,ym,qt[curr].p2.z});
    qt[curr].children[7]=init_node((Vec3){xm,ym,zm},qt[curr].p2);
}

void insert_body(int v,int body_idx){
    if(v==-1 || body_idx==-1) return;
    
    // aggreagte+_vals(v,body_idx);
    
    if(is_leaf(v) && qt[v].body_idx==-1){
        qt[v].body_idx=body_idx;
        return;
    }

    
}

int main(){
    // int n1=init_node((Vec3){0.0f,0.0f,0.0f},(Vec3){5.0f,5.0f,5.0f});
    // split_node(n1);
    // bods[0].p=(Vec3){4.0f,2.5f,4.0f};
    // char *strs[8]={"BOTTOM_LEFT_BACK","BOTTOM_RIGHT_BACK","TOP_LEFT_BACK","TOP_RIGHT_BACK","BOTTOM_LEFT_FRONT","BOTTOM_RIGHT_FRONT","TOP_LEFT_FRONT","TOP_RIGHT_FRONT"};
    // int x=get_cube_num(n1,0);
    // printf(strs[x]); printf("\n");



    return 0;
}