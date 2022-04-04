#ifndef __SPLINE__
#define __SPLINE__
#define GLM_FORCE_SIMD_AVX2
#include <vector>
#include <glm/glm.hpp>
#include <iostream>
#include <gsl/gsl_linalg.h>
#include <stdlib.h>
#include <memory.h>

enum SPLINETYPE{
    BSPLINE,
    CATMULL_ROM
};

template <typename T>
class ISpline {
public:
    std::vector<T> points;
    std::vector<T> cPoints;
    ISpline(){this->cPoints.clear();}
    void _interpol(int N,T (splineFunc(std::vector<T>,float))){
        this->points.clear();
        int numSegment = this->cPoints.size()/4;
        for(int i=0;i<numSegment;i++){
            auto segCP = std::vector<T>(
                    this->cPoints.begin()+i*4,
                    this->cPoints.begin()+(i+1)*4
                );
            std::vector<T> d1;
            std::vector<T> d2;
            T d3;
            for(int j=0;j<N;j++){
                this->points.push_back(splineFunc(segCP,(float)j/(float)N));
            }
        }
    }
    virtual void pointConversion() = 0;
    inline int idxConversion(int idx, int N){
        return (idx+N)%N;
    }
};

template <typename T>
class BSpline : public ISpline<T>{
public:
    BSpline(std::vector<T> _points):ISpline<T>(){
        this->points = _points;
        this->pointConversion();
    }
    static T bSplineCurve(std::vector<T> cp, float t){
        return (1.0f/6.0f)*
            ((1.0f-t)*(1.0f-t)*(1.0f-t)*cp[0]
            + (3.0f*t*t*t-6.0f*t*t+4)        * cp[1]
            + (-3.0f*t*t*t+3.0f*t*t+3.0f*t+1.0f)* cp[2]
            + (t*t*t)                        * cp[3]); 
    }
    void pointConversion(){
        gsl_vector* x;
        gsl_vector* b;
        int numPoints = this->points.size();
        int dim = this->points[0].length();
        double*  _row = new double[numPoints];
        gsl_matrix* mat = gsl_matrix_alloc(numPoints,numPoints);

        auto tmp_points = std::vector<T>(this->points);
        this->cPoints.clear();


        x = gsl_vector_alloc(numPoints);
        b = gsl_vector_alloc(numPoints);

        _row[0] = 1.0/6.0;
        _row[1] = 4.0/6.0;
        _row[2] = 1.0/6.0;
        for(int i=3;i<numPoints;i++) _row[i] = 0.0;
        for(int i=0; i<numPoints;i++){
            int j =1-i+numPoints;
            int _j = this->idxConversion(j,numPoints);
            int k = 0;
            do{
                gsl_matrix_set(mat,i,k,_row[this->idxConversion(j,numPoints)]);
                j++;
                k++;
            }while(this->idxConversion(j,numPoints)!=_j);
        }
        if(gsl_linalg_ldlt_decomp(mat) == GSL_EDOM){
            std::cout<<"[ERROR] : SINGULAR MATRIX DECOMPOSITION"<<std::endl;
            exit(-1);
        }
        delete _row;
        for(int i=0; i<dim;i++){
            for(int j=0; j<numPoints; j++){
                gsl_vector_set(b,j,(double)(this->points[j][i]));
            }
            gsl_linalg_ldlt_solve(mat,b,x);
            for(int j=0; j<numPoints;j++){
                (tmp_points[j])[i] = (float)(x->data[j]);
            }
        }
        for(int i=0; i<numPoints; i++){
            this->cPoints.push_back(tmp_points[this->idxConversion(i,numPoints)]);
            this->cPoints.push_back(tmp_points[this->idxConversion(i+1,numPoints)]);
            this->cPoints.push_back(tmp_points[this->idxConversion(i+2,numPoints)]);
            this->cPoints.push_back(tmp_points[this->idxConversion(i+3,numPoints)]);
        }
        gsl_vector_free(x);
        gsl_vector_free(b);
        gsl_matrix_free(mat);
    }
    void interpol(int N){
        this->_interpol(N,this->bSplineCurve);
    }
};

template<typename T>
class CATMULL_ROM_SPLINE : public ISpline<T>{
public:
    CATMULL_ROM_SPLINE(std::vector<T> _points):ISpline<T>(){
        this->points = _points;
        this->pointConversion();
    }
    static T bezierCurve(std::vector<T> cp, float t){
        return (1.0f-t)*(1.0f-t)*(1.0f-t)*cp[0]
            + 3.0f*t*(1-t)*(1-t)*cp[1]
            + 3.0f*t*  t  *(1-t)*cp[2]
            +      t*  t  *  t  *cp[3]; 
    }
    void pointConversion(){
        this->cPoints.clear();
        std::vector<T> dPoints;
        dPoints.clear();
        const int N = this->points.size();
        for(int i=0; i < N; i++){
            auto dev = this->points[this->idxConversion(i+1,N)]
                        - this->points[this->idxConversion(i-1,N)];
            dev /= 6.0f;
            dPoints.push_back(dev);
        }
        for(int i=0; i < N; i++){
            auto idx = this->idxConversion(i,N);
            auto idx_ = this->idxConversion(i+1,N);
            this->cPoints.push_back(this->points[idx]);
            this->cPoints.push_back(this->points[idx]+dPoints[idx]);
            this->cPoints.push_back(this->points[idx_]-dPoints[idx_]);
            this->cPoints.push_back(this->points[idx_]);
        }
    }
    void interpol(int N){
        this->_interpol(N,this->bezierCurve);
    }
};

template<typename T>
class CATMULL_ROM_SPLINE_OPEN : public ISpline<T>{
public:
    CATMULL_ROM_SPLINE_OPEN(std::vector<T> _points):ISpline<T>(){
        this->points = _points;
        this->pointConversion();
    }
    static T bezierCurve(std::vector<T> cp, float t){
        return (1.0f-t)*(1.0f-t)*(1.0f-t)*cp[0]
            + 3.0f*t*(1-t)*(1-t)*cp[1]
            + 3.0f*t*  t  *(1-t)*cp[2]
            +      t*  t  *  t  *cp[3]; 
    }
    void pointConversion(){
        this->cPoints.clear();
        std::vector<T> dPoints;
        dPoints.clear();
        const int N = this->points.size();
        dPoints.push_back(T());
        for(int i=1; i < N-1; i++){
            auto dev = this->points[i+1]
                        - this->points[i-1];
            dev /= 6.0f;
            dPoints.push_back(dev);
        }
        this->cPoints.push_back(this->points[0]);
        this->cPoints.push_back((this->points[0]+this->points[1])*0.5f);
        for(int i=1; i < N-1; i++){
            auto idx = i;
            auto idx_ = i+1;
            this->cPoints.push_back(this->points[idx]-dPoints[idx]);
            this->cPoints.push_back(this->points[idx]);
            this->cPoints.push_back(this->points[idx]);
            this->cPoints.push_back(this->points[idx]+dPoints[idx]);
        }
        this->cPoints.push_back((this->points[N-2]+this->points[N-1])*0.5f);
        this->cPoints.push_back(this->points[N-1]);
    }
    void interpol(int N){
        this->_interpol(N,this->bezierCurve);
    }
};
#endif