kernel void GaussJordanDiv(global float* matrix, global float* I,  int n, int y, int rp)
{
	float div= matrix[y*n+y];
	int id=get_global_id(0);
	int x;
	for(int p=0;p<rp;p++)
	{ 
		x= id*rp+p;
		if(x<n)
		{ 
			if(x!=y)
			{ 
				if(div!=0.0)
				{
					I[y*n+x] = I[y*n+x]/div;
					matrix[y*n+x] = matrix[y*n+x]/div;
				}
				else
				{
					for(int i=0;i<n;i++)
					{ 
						if(matrix[i*n+y]!=0.0)
						{
							matrix[y*n+x] = (matrix[y*n+x]+matrix[i*n+x])/matrix[i*n+y];
							I[y*n+x] = (I[y*n+x]+I[i*n+x])/matrix[i*n+y];
							return;
						}
					}
				}
			}
			if(x==y && div==0.0)
			{
				for(int i=0;i<n;i++)
				{ 
					if(matrix[i*n+y]!=0.0)
					{
						matrix[y*n+x] = matrix[i*n+y];
					}
				}
			}
		}
	}
}
kernel void GaussJordanDiagDiv(global float* matrix, global float* I,  int n, int y)
{
	I[y*n+y] = I[y*n+y]/matrix[y*n+y];
	matrix[y*n+y] = 1.0;
}
kernel void GaussJordanElim(global float* matrix, global float* I,  int n, int i, int mp)
{
	int idx= get_global_id(0);
	int idy= get_global_id(1);
	int x,y;
	for(int p=0;p<mp;p++)
	{ 
		y= idy*mp+p;
		float pivot=matrix[y*n+i];
		for(int pp=0;pp<mp;pp++)
		{ 
			x= idx*mp+pp;
			if(x<n && y<n)
			{ 
				if(y!=i&& x!=i)
				{ 
					I[y*n+x] = I[y*n+x]-(I[i*n+x]*pivot);
					matrix[y*n+x] = matrix[y*n+x]-(matrix[i*n+x]*pivot);
				}
			}
		}
	}
}
kernel void GaussJordanElimPiv(global float* matrix, global float* I,  int n, int i, int mp)
{
	int x= i;
	int idy= get_global_id(0);
	int y;
	for(int p=0;p<mp;p++)
	{ 
		y= idy*mp+p;
		if(y!=i && y<n)
		{ 
			float pivot=matrix[y*n+i];
			I[y*n+x] = I[y*n+x]-(I[i*n+x]*pivot);
			matrix[y*n+x] = matrix[y*n+x]-(matrix[i*n+x]*pivot);
		}
	}
}

kernel void GaussJordanMain(global float* matrix, global float* I,  int n, queue_t que, int divPort, int elimPort, int elimPivPort)
{

	ndrange_t  divRange=ndrange_1D(n/divPort+1);
	ndrange_t  divDiagRange=ndrange_1D(1);
	size_t elim_global [2]={n/elimPort+1,n/elimPort+1};
	ndrange_t  elimRange=ndrange_2D(elim_global);
	ndrange_t  elimRangePiv=ndrange_1D(n/elimPivPort+1);
	clk_event_t Events[4];
	enqueue_kernel(que,CLK_ENQUEUE_FLAGS_NO_WAIT,divRange,0,NULL,&Events[0],^{GaussJordanDiv(matrix,I,n,0,divPort);});
	enqueue_kernel(que,CLK_ENQUEUE_FLAGS_NO_WAIT,divDiagRange,1,&Events[0],&Events[1],^{GaussJordanDiagDiv(matrix,I,n,0);});
	release_event(Events[0]);
	enqueue_kernel(que,CLK_ENQUEUE_FLAGS_NO_WAIT,elimRange,1,&Events[1],&Events[2],^{GaussJordanElim(matrix,I,n,0,elimPort);});
	release_event(Events[1]);
	enqueue_kernel(que,CLK_ENQUEUE_FLAGS_NO_WAIT,elimRangePiv,1,&Events[2],&Events[3],^{GaussJordanElimPiv(matrix,I,n,0,elimPivPort);});
	release_event(Events[2]);
	for(int i=1;i<n;i++)
	{ 
		enqueue_kernel(que,CLK_ENQUEUE_FLAGS_NO_WAIT,divRange,1,&Events[3],&Events[0],^{GaussJordanDiv(matrix,I,n,i,divPort);});
		release_event(Events[3]);
		enqueue_kernel(que,CLK_ENQUEUE_FLAGS_NO_WAIT,divDiagRange,1,&Events[0],&Events[1],^{GaussJordanDiagDiv(matrix,I,n,i);});
		release_event(Events[0]);
		enqueue_kernel(que,CLK_ENQUEUE_FLAGS_NO_WAIT,elimRange,1,&Events[1],&Events[2],^{GaussJordanElim(matrix,I,n,i,elimPort);});
		release_event(Events[1]);
		enqueue_kernel(que,CLK_ENQUEUE_FLAGS_NO_WAIT,elimRangePiv,1,&Events[2],&Events[3],^{GaussJordanElimPiv(matrix,I,n,i,elimPivPort);});
		release_event(Events[2]);
	}
	//enqueue_kernel(que,CLK_ENQUEUE_FLAGS_NO_WAIT,divDiagRange,^{GaussJordanDivOpt(matrix,I,n,0);});
}