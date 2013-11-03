/*
 * Open source copyright declaration based on BSD open source template:
 * http://www.opensource.org/licenses/bsd-license.php
 *
 * This file is part of the OP2 distribution.
 *
 * Copyright (c) 2011, Mike Giles and others. Please see the AUTHORS file in
 * the main source directory for a full list of copyright holders.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * The name of Mike Giles may not be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY Mike Giles ''AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL Mike Giles BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

//
// This file implements the OP2 user-level functions for the CUDA backend
//

#if defined(__APPLE__) || defined(__MACOSX)
    #include <OpenCL/cl.h>
#else
    #include <CL/cl.h>
#endif
//#include <cuda.h>
//#include <cuda_runtime.h>
//#include <cuda_runtime_api.h>

#include <op_lib_c.h>
#include <op_opencl_rt_support.h>
#include <op_rt_support.h>
#include <op_opencl_core.h>

extern op_opencl_core OP_opencl_core;

//
// OpenCL-specific OP2 functions
//

void
op_init ( int argc, char ** argv, int diags )
{
  op_init_core ( argc, argv, diags );

//#if CUDART_VERSION < 3020
//#error : "must be compiled using CUDA 3.2 or later"
//#endif

//#ifdef CUDA_NO_SM_13_DOUBLE_INTRINSICS
//#warning : " *** no support for double precision arithmetic *** "
//#endif

  openclDeviceInit ( argc, argv );

//
// The following call is only made in the C version of OP2,
// as it causes memory trashing when called from Fortran.
// \warning add -DSET_CUDA_CACHE_CONFIG to compiling line
// for this file when implementing C OP2.
//

// There is no such option in OpenCL!!!
//#ifdef SET_CUDA_CACHE_CONFIG
//  cutilSafeCall ( cudaThreadSetCacheConfig ( cudaFuncCachePreferShared ) );
//#endif

  printf ( "\n 16/48 L1/shared Option in OpenCL won't be available probably.\n" );
}

op_dat
op_decl_dat_char ( op_set set, int dim, char const *type, int size,
              char * data, char const * name )
{
  op_dat dat = op_decl_dat_core ( set, dim, type, size, data, name );

  //transpose data
  if (strstr( type, ":soa")!= NULL) {
    char *temp_data = (char *)malloc(dat->size*set->size*sizeof(char));
    int element_size = dat->size/dat->dim;
    for (int i = 0; i < dat->dim; i++) {
      for (int j = 0; j < set->size; j++) {
        for (int c = 0; c < element_size; c++) {
          temp_data[element_size*i*set->size + element_size*j + c] = data[dat->size*j+element_size*i+c];
        }
      }
    }
    op_cpHostToDevice ( ( void ** ) &( dat->data_d ),
                          ( void ** ) &( temp_data ), dat->size * set->size );
    free(temp_data);
  } else {
    op_cpHostToDevice ( ( void ** ) &( dat->data_d ),
                        ( void ** ) &( dat->data ), dat->size * set->size );
  }

  return dat;
}


op_dat
op_decl_dat_temp_char ( op_set set, int dim, char const *type, int size, char const * name )
{
//  char* data = NULL;
//  op_dat dat = op_decl_dat_temp_core ( set, dim, type, size, data, name );
//
//  dat->data = (char*) calloc(set->size*dim*size, 1); //initialize data bits to 0
//  dat-> user_managed = 0;
//
//  //transpose data
//  if (strstr( type, ":soa")!= NULL) {
//    char *temp_data = (char *)malloc(dat->size*set->size*sizeof(char));
//    int element_size = dat->size/dat->dim;
//    for (int i = 0; i < dat->dim; i++) {
//      for (int j = 0; j < set->size; j++) {
//        for (int c = 0; c < element_size; c++) {
//          temp_data[element_size*i*set->size + element_size*j + c] = data[dat->size*j+element_size*i+c];
//        }
//      }
//    }
//    op_cpHostToDevice ( ( void ** ) &( dat->data_d ),
//                          ( void ** ) &( temp_data ), dat->size * set->size );
//    free(temp_data);
//  } else {
//    op_cpHostToDevice ( ( void ** ) &( dat->data_d ),
//                        ( void ** ) &( dat->data ), dat->size * set->size );
//  }
//
//  return dat;
  return 0;
}

int op_free_dat_temp_char ( op_dat dat )
{
  //free data on device
//  cutilSafeCall (cudaFree(dat->data_d));

  printf("op_free_dat_temp_char ( op_dat dat )\n");
  clReleaseMemObject(*((cl_mem*)(dat->data_d)));

  return op_free_dat_temp_core (dat);
}

op_set
op_decl_set ( int size, char const * name )
{
  return op_decl_set_core ( size, name );
}

op_map
op_decl_map ( op_set from, op_set to, int dim, int * imap, char const * name )
{
  op_map map = op_decl_map_core ( from, to, dim, imap, name );
  int set_size = map->from->size;
  int *temp_map = (int *)malloc(map->dim*set_size*sizeof(int));
  for (int i = 0; i < map->dim; i++) {
    for (int j = 0; j < set_size; j++) {
      temp_map[i*set_size + j] = map->map[map->dim*j+i];
    }
  }
  op_cpHostToDevice ( ( void ** ) &( map->map_d ),
                      ( void ** ) &( temp_map ), map->dim * set_size * sizeof(int) );
  free(temp_map);
  return map;
}

op_arg
op_arg_dat ( op_dat dat, int idx, op_map map, int dim, char const * type,
             op_access acc )
{
  return op_arg_dat_core ( dat, idx, map, dim, type, acc );
}

op_arg
op_arg_gbl_char ( char * data, int dim, const char *type, int size, op_access acc )
{
  return op_arg_gbl_core ( data, dim, type, size, acc );
}

//
// This function is defined in the generated master kernel file
// so that it is possible to check on the runtime size of the
// data in cases where it is not known at compile time
//


//op_decl_const_char ( dim, type, sizeof ( T ), (char *) data, name );
void
op_decl_const_char ( int dim, char const * type, int size, char * dat,
                     char const * name )
{
#warning "const_d is not kept track of, therefore it will not be freed up!"
  // Add constant to constant array
//  cl_mem const_tmp[OP_opencl_core.n_constants+1];
  cl_mem *const_tmp;
  const_tmp = (cl_mem*) malloc((OP_opencl_core.n_constants)*sizeof(cl_mem));
  for(int i=0; i<OP_opencl_core.n_constants; i++)
    const_tmp[i] = OP_opencl_core.constant[i];

  cl_int ret = 0;
  OP_opencl_core.n_constants++;
  OP_opencl_core.constant = (cl_mem*) malloc((OP_opencl_core.n_constants)*sizeof(cl_mem));

  for(int i=0; i<OP_opencl_core.n_constants-1; i++)
    OP_opencl_core.constant[i] = const_tmp[i];

  OP_opencl_core.constant[OP_opencl_core.n_constants-1] = clCreateBuffer(OP_opencl_core.context, CL_MEM_READ_ONLY, dim*size, NULL, &ret);
  clSafeCall( ret );

  clSafeCall( clEnqueueWriteBuffer(OP_opencl_core.command_queue, OP_opencl_core.constant[OP_opencl_core.n_constants-1], CL_TRUE, 0, dim*size, (void*) dat, 0, NULL, NULL) );
  clSafeCall( clFlush(OP_opencl_core.command_queue) );
  clSafeCall( clFinish(OP_opencl_core.command_queue) );

//  free(OP_opencl_core.constant);


//  cutilSafeCall ( cudaMemcpyToSymbol ( name, dat, dim * size, 0,
//                                       cudaMemcpyHostToDevice ) );
}


int op_get_size(op_set set)
{
  return set->size;
}

void op_printf(const char* format, ...)
{
  va_list argptr;
  va_start(argptr, format);
  vprintf(format, argptr);
  va_end(argptr);
}

void op_timers(double * cpu, double * et)
{
  op_timers_core(cpu,et);
}

void op_exit()
{
  op_opencl_exit();            // frees dat_d memory
  op_rt_exit();              // frees plan memory
  op_exit_core();            // frees lib core variables
}

void op_timing_output()
{
  op_timing_output_core();
}
