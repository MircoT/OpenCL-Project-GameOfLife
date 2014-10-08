inline int modulo(int a, int b)
{
	const int result = a % b;
	return result >= 0 ? result : result + b;
}

__kernel void gameOfLifeKernel(int rows, int columns, __global int *matrix, __global int *result)
{
    //Get the index of the work-item
    int globalId = get_global_id(0);
    
	short neighbors = 0;
    
	// item = [x + y*w]
    int x = globalId%columns;
    int y = globalId/columns;
    
	if (matrix[modulo(x - 1, columns) + y*columns] == 1)
		neighbors++;
	if (matrix[modulo(x + 1, columns) + y*columns] == 1)
		neighbors++;
	if (matrix[x + modulo(y - 1, rows)*columns] == 1)
		neighbors++;
	if (matrix[x + modulo(y + 1, rows)*columns] == 1)
		neighbors++;
	if (matrix[modulo(x + 1, columns) + modulo(y + 1, rows)*columns] == 1)
		neighbors++;
	if (matrix[modulo(x + 1, columns) + modulo(y - 1, rows)*columns] == 1)
		neighbors++;
	if (matrix[modulo(x - 1, columns) + modulo(y + 1, rows)*columns] == 1)
		neighbors++;
	if (matrix[modulo(x - 1, columns) + modulo(y - 1, rows)*columns] == 1)
		neighbors++;
    
	if (matrix[globalId] == 0)
    {
		if (neighbors == 3)
			result[globalId] = 1;
		else
			result[globalId] = 0;
    }
    else if (matrix[globalId] == 1)
    {
        if (neighbors < 2)
			result[globalId] = 0;
        else if (neighbors == 2 || neighbors == 3)
			result[globalId] = 1;
        else if (neighbors > 3)
			result[globalId] = 0;;
    }
}
