#include <vector>
#include <stdlib.h>
#include <iostream>

#pragma once

template<class T>
class Matrix
{
	template<class D>
	friend std::ostream& operator<<(std::ostream& os, const Matrix<D> &matrix);
    
	size_t w;
	size_t h;
	std::vector < T > matrix;
    
public:
    
	Matrix(size_t w, size_t h)
    :w(w)
    ,h(h)
	{
		matrix.resize(w*h, (T) 0);
	}
    
	T& operator () (size_t x, size_t y)
	{
		return matrix[x + y*w];
	}
    
	T operator () (size_t x, size_t y) const
	{
		return matrix[x + y*w];
	}
    
	void swap(Matrix <T> &buffer)
	{
		matrix.swap(buffer.matrix);
	}
    
	void to_zero()
	{
		std::fill(matrix.begin(), matrix.end(), 0);
	}
    
	T* get_row(size_t y)
	{
		return &matrix[y*w];
	}
    
	inline size_t width(){ return w; }
	inline size_t height(){ return h; }
};

template<class T>
std::ostream& operator<<(std::ostream& os, const Matrix<T> &matrix)
{
	for (size_t row = 0; row != matrix.h; ++row)
	{
		for (size_t column = 0; column != matrix.w; ++column)
		{
            os << matrix(column, row) << ' ';
            /*if (matrix(column, row) == 0)
             os << ' ';
             else
             os << '#';*/
		}
		os << std::endl;
	}
	return os;
};

/*template<>
 std::ostream& operator<<(std::ostream& os, const Matrix<unsigned char> &matrix)
 {
 for (size_t row = 0; row != matrix.h; ++row)
 {
 for (size_t column = 0; column != matrix.w; ++column)
 {
 os << (int) matrix(column, row);
 }
 os << std::endl;
 }
 return os;
 };*/

inline int modulo(unsigned long a, unsigned long b)
{
	const int result = (int)a % (int)b;
	return result >= 0 ? result : result + (int)b;
}

class GameOfLife
{
	
    Matrix <int> matrix;
    Matrix <int> buffer;
    
public:
    int getNeighbors(unsigned long column, unsigned long row)
    {
        int counter = 0;
        if (matrix(modulo(column - 1, matrix.width()), row) == 1)
        counter++;
        if (matrix(modulo(column + 1, matrix.width()), row) == 1)
        counter++;
        if (matrix(column, modulo(row - 1, matrix.height())) == 1)
        counter++;
        if (matrix(column, modulo(row + 1, matrix.height())) == 1)
        counter++;
        if (matrix(modulo(column + 1, matrix.width()), modulo(row + 1, matrix.height())) == 1)
        counter++;
        if (matrix(modulo(column + 1, matrix.width()), modulo(row - 1, matrix.height())) == 1)
        counter++;
        if (matrix(modulo(column - 1, matrix.width()), modulo(row + 1, matrix.height())) == 1)
        counter++;
        if (matrix(modulo(column - 1, matrix.width()), modulo(row - 1, matrix.height())) == 1)
        counter++;
        return counter;
    }
    
    int * getRow(size_t y)
    {
        return matrix.get_row(y);
    }
    
    void step()
    {
        
        for (size_t row = 0; row != matrix.height(); ++row)
        {
            for (size_t column = 0; column != matrix.width(); ++column)
            {
                int neighbors = getNeighbors(column, row);
                if (matrix(column, row) == 0)
                {
                    if (neighbors == 3)
                    buffer(column, row) = 1;
                }
                else if (matrix(column, row) == 1)
                {
                    if (neighbors < 2)
                    buffer(column, row) = 0;
                    else if (neighbors == 2 || neighbors == 3)
                    buffer(column, row) = 1;
                    else if (neighbors > 3)
                    buffer(column, row) = 0;
                }
            }
        }
        
        matrix.swap(buffer);
        buffer.to_zero();
    }
    
    void setItem(int x, int y, int val)
    {
        matrix(x, y) = val;
    }
    
    void print()
    {
        std::cout << matrix;
    }
    
    GameOfLife(int width, int height)
    : matrix(width, height)
    , buffer(width, height)
    {
    }
    
};

