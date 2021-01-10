#ifndef __VECTOR_H
#define __VECTOR_H

// ---------------------------------------------------------------------------------------------------------------------
// Includes
// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
// Class
// ---------------------------------------------------------------------------------------------------------------------
class PVector
{
    public:
    float x;
    float y;
     
        PVector(void);
        PVector(float x, float y);
        PVector& add(PVector& v);
        PVector& sub(PVector& v);
        PVector& mult(float value);
        PVector& div(float value);
        PVector& normalize(void);
        float mag(void);
        float heading(void);
        
    private:
};

// ---------------------------------------------------------------------------------------------------------------------

// ---------------------------------------------------------------------------------------------------------------------
// Exported functions
// ---------------------------------------------------------------------------------------------------------------------

// ---------------------------------------------------------------------------------------------------------------------

#endif /* __VECTOR_H */