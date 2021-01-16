// ---------------------------------------------------------------------------------------------------------------------
// Includes
// ---------------------------------------------------------------------------------------------------------------------

#include "app.h"
#include "vector.hpp"
#include <list>

extern "C" {
    #include "math.h"
    #include <time.h>
    #include <stdlib.h>
    #include "rtree.h"
}

// ---------------------------------------------------------------------------------------------------------------------
// Defines/macros
// ---------------------------------------------------------------------------------------------------------------------
#define LCD_WIDTH                       240
#define LCD_HEIGHT                      320

#define CIRCLE_RADIUS                   6
#define NUMBER_OF_PARTICLES             80
#define INITIAL_DIST_BETWEEN_PARTS      8
#define MAX_PARTICLES_PER_ROW           (LCD_WIDTH / (2 * (CIRCLE_RADIUS + INITIAL_DIST_BETWEEN_PARTS)))
#define MAX_PARTICLES_PER_COL           (LCD_HEIGHT / (2 * (CIRCLE_RADIUS + INITIAL_DIST_BETWEEN_PARTS)))
#define MAX_PARTICLES                   (                                                                              \
                                            (LCD_WIDTH * LCD_HEIGHT)                                                   \
                                            /                                                                          \
                                            (                                                                          \
                                                (2 * (CIRCLE_RADIUS + INITIAL_DIST_BETWEEN_PARTS))                     \
                                                *                                                                      \
                                                (2 * (CIRCLE_RADIUS + INITIAL_DIST_BETWEEN_PARTS))                     \
                                            )                                                                          \
                                        )

#define MAX_FRICTION_RAND_MOD           10
#define MAX_FRICTION                    0.1f
#define REFRESH_RATE                    60 //Hz
#define REFRESH_PERIOD                  ((uint32_t)((1.0/REFRESH_RATE) * 1000))
#define MIN_INITIAL_SPEED               150
#define MAX_INITIAL_SPEED               200 

#if NUMBER_OF_PARTICLES > MAX_PARTICLES
#error Number of particles is greater than maximum number
#endif


// ---------------------------------------------------------------------------------------------------------------------
// Private typedefs
// ---------------------------------------------------------------------------------------------------------------------



// ---------------------------------------------------------------------------------------------------------------------
// Private constants
// ---------------------------------------------------------------------------------------------------------------------
static const uint16_t colors[] = 
{
    LCD_COLOR_WHITE, LCD_COLOR_GREY, LCD_COLOR_BLUE, LCD_COLOR_BLUE2, LCD_COLOR_RED, LCD_COLOR_MAGENTA, LCD_COLOR_GREEN, 
    LCD_COLOR_CYAN, LCD_COLOR_YELLOW
};


// ---------------------------------------------------------------------------------------------------------------------
// Private variables
// ---------------------------------------------------------------------------------------------------------------------
static struct rtree *tr;
static Particle_t particles[NUMBER_OF_PARTICLES];

// ---------------------------------------------------------------------------------------------------------------------
// Private prototypes
// ---------------------------------------------------------------------------------------------------------------------


// ---------------------------------------------------------------------------------------------------------------------
// Private functions
// ---------------------------------------------------------------------------------------------------------------------
static void initialize_particles(void)
{
    srand(time(0));
    memset(particles, 0, sizeof(particles));
    tr = rtree_new(sizeof(struct city*), 2);
    
    for(int i = 0; i < NUMBER_OF_PARTICLES; i++)
    {
        Particle_t* part = &particles[i];
        part->used = 1;
        part->color = colors[rand() % (sizeof(colors)/sizeof(colors[0]))];
        
        part->vx = (rand() % MAX_INITIAL_SPEED) * ((rand() % 2 == 0) ? -1 : 1);
        part->vx = MAX(part->vx, MIN_INITIAL_SPEED) * (1.0/REFRESH_RATE);
        part->vy = (rand() % MAX_INITIAL_SPEED) * ((rand() % 2 == 0) ? -1 : 1);
        part->vy = MAX(part->vy, MIN_INITIAL_SPEED) * (1.0/REFRESH_RATE);
        
        part->ax = (rand() % MAX_FRICTION_RAND_MOD);
        part->ax = MAX_FRICTION/REFRESH_RATE / MAX(part->ax, 1);
        
        part->ay = (rand() % MAX_FRICTION_RAND_MOD);
        part->ay = MAX_FRICTION/REFRESH_RATE / MAX(part->ay, 1);
        
        part->x = CIRCLE_RADIUS + (i % MAX_PARTICLES_PER_ROW) * (2 * (CIRCLE_RADIUS + INITIAL_DIST_BETWEEN_PARTS));
        part->y = CIRCLE_RADIUS + (i / MAX_PARTICLES_PER_ROW) * (2 * (CIRCLE_RADIUS + INITIAL_DIST_BETWEEN_PARTS));
        
        double rect[] = { 
            part->x - CIRCLE_RADIUS, part->y - CIRCLE_RADIUS, part->x + CIRCLE_RADIUS, part->y + CIRCLE_RADIUS 
        };
        
        rtree_insert(tr, rect, &part);
    }
}
// ---------------------------------------------------------------------------------------------------------------------

static void check_boundaries_collision(Particle_t* part)
{
    if(part->x > LCD_WIDTH - CIRCLE_RADIUS)
    {
        part->x = LCD_WIDTH - CIRCLE_RADIUS;
        part->vx = -part->vx;
    }
    else if(part->x < CIRCLE_RADIUS)
    {
        part->x = CIRCLE_RADIUS;
        part->vx = -part->vx;
    }
    
    if(part->y > LCD_HEIGHT - CIRCLE_RADIUS)
    {
        part->y = LCD_HEIGHT - CIRCLE_RADIUS;
        part->vy = -part->vy;
    }
    else if(part->y < CIRCLE_RADIUS)
    {
        part->y = CIRCLE_RADIUS;
        part->vy = -part->vy;
    }
}
// ---------------------------------------------------------------------------------------------------------------------

static bool check_particle_collision(const double *rect, const void *item, void *udata)
{
    int** args = (int**)udata;
    
    Particle_t* part = (Particle_t*)*args[0];
    list<Particle_t*>* changeList = (list<Particle_t*>*)args[1];
    Particle_t* temp = *((Particle_t**)item);

    if(part != temp)
    {
        PVector position(part->x, part->y);
        PVector otherPosition(temp->x, temp->y);
        
        PVector distanceVect = PVector(position.x - otherPosition.x, position.y - otherPosition.y);
        float distanceVectMag = distanceVect.mag();
        float minDistance = 2 * CIRCLE_RADIUS;
        if(distanceVectMag < minDistance)
        {
            float distanceCorrection = (minDistance - distanceVectMag) / 2.0;
            PVector d = PVector(distanceVect.x, distanceVect.y);
            PVector correctionVector = d.normalize().mult(distanceCorrection);
            
            otherPosition.sub(correctionVector);
            position.add(correctionVector);
            
            distanceVect = PVector(position.x - otherPosition.x, position.y - otherPosition.y);
            
            float theta  = distanceVect.heading();
            float sine = sin(theta);
            float cosine = cos(theta);
            
            PVector bTemp[2];
            
            bTemp[1].x  = cosine * distanceVect.x + sine * distanceVect.y;
            bTemp[1].y  = cosine * distanceVect.y - sine * distanceVect.x;
            
            PVector vTemp[2];
            
            vTemp[0].x  = cosine * part->vx + sine * part->vy;
            vTemp[0].y  = cosine * part->vy - sine * part->vx;
            vTemp[1].x  = cosine * temp->vx + sine * temp->vy;
            vTemp[1].y  = cosine * temp->vy - sine * temp->vx;
            
            PVector vFinal[2];
            
            //vFinal[0].x = ((m - other.m) * vTemp[0].x + 2 * other.m * vTemp[1].x) / (m + other.m);
            vFinal[0].x = vTemp[1].x;
            vFinal[0].y = vTemp[0].y;
            
            //vFinal[1].x = ((other.m - m) * vTemp[1].x + 2 * m * vTemp[0].x) / (m + other.m);
            vFinal[1].x = vTemp[0].x;
            vFinal[1].y = vTemp[1].y;
            
            bTemp[0].x += vFinal[0].x;
            bTemp[1].x += vFinal[1].x;
            
            PVector bFinal[2];
            
            bFinal[0].x = cosine * bTemp[0].x - sine * bTemp[0].y;
            bFinal[0].y = cosine * bTemp[0].y + sine * bTemp[0].x;
            bFinal[1].x = cosine * bTemp[1].x - sine * bTemp[1].y;
            bFinal[1].y = cosine * bTemp[1].y + sine * bTemp[1].x;
            
            part->vx = cosine * vFinal[0].x - sine * vFinal[0].y;
            part->vy = cosine * vFinal[0].y + sine * vFinal[0].x;
            temp->vx = cosine * vFinal[1].x - sine * vFinal[1].y;
            temp->vy = cosine * vFinal[1].y + sine * vFinal[1].x;
            
            part->x = position.x;
            part->y = position.y;
            
            temp->x = otherPosition.x;
            temp->y = otherPosition.y;
            
            changeList->push_front(temp);
        }
    }
    
    return true;
}
// ---------------------------------------------------------------------------------------------------------------------

static void update_particles(void)
{
    for(int i = 0; i < NUMBER_OF_PARTICLES; i++)
    {
        Particle_t* part = (Particle_t*)&particles[i];
        part->x += part->vx;
        part->y += part->vy;
        check_boundaries_collision(part);
        
        double rect[] = { 
            part->x - 2 * CIRCLE_RADIUS, 
            part->y - 2 * CIRCLE_RADIUS, 
            part->x + 2 * CIRCLE_RADIUS, 
            part->y + 2 * CIRCLE_RADIUS 
        };
        
        list<Particle_t*> changeList;
        changeList.push_front(part);
        
        int* args[] = {
            (int*)&part,
            (int*)&changeList
        };
        
        rtree_search(tr, rect, check_particle_collision, &args);
        
        for(list<Particle_t*>::iterator it = changeList.begin(); it != changeList.end(); it++)
        {
            Particle_t* partTemp = (Particle_t*)*it;
            double rect[] = { 
                partTemp->x - CIRCLE_RADIUS, partTemp->y - CIRCLE_RADIUS, partTemp->x + CIRCLE_RADIUS, partTemp->y + CIRCLE_RADIUS 
            };
            rtree_delete(tr, rect, &partTemp);
            rtree_insert(tr, rect, &partTemp);
        }
        
        part->vx *= (1.0 - part->ax);
        part->vy *= (1.0 - part->ay);
    }
}
// ---------------------------------------------------------------------------------------------------------------------

static void draw_particles(bool clear)
{
    if(clear)
        LCD_Clear(LCD_COLOR_BLACK);
    
    for(int i = 0; i < NUMBER_OF_PARTICLES; i++)
    {
        Particle_t* part = (Particle_t*)&particles[i];
        LCD_SetTextColor(part->color);
        LCD_DrawCircle((uint16_t)part->x, (uint16_t)part->y, CIRCLE_RADIUS);
    }
}
// ---------------------------------------------------------------------------------------------------------------------

// ---------------------------------------------------------------------------------------------------------------------
// Public functions
// ---------------------------------------------------------------------------------------------------------------------
void app_init(void)
{
    initialize_particles();
    draw_particles(true);
}
// ---------------------------------------------------------------------------------------------------------------------

void app_update(void)
{
    draw_particles(true);
    update_particles();
    delayMiliSecs(REFRESH_PERIOD);
}
// ---------------------------------------------------------------------------------------------------------------------
