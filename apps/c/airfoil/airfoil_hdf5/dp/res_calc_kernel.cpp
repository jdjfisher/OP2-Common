//
// auto-generated by op2.py on 2014-03-11 14:18
//

//user function
#include "res_calc.h"
#ifdef VECTORIZE
inline void res_calc_vec(const doublev *x1, const doublev *x2, const doublev *q1, const doublev *q2,
                     const doublev *adt1, const doublev *adt2, doublev *res1, doublev *res2) {
  doublev dx,dy,mu, ri, p1,vol1, p2,vol2, f;

  dx = x1[0] - x2[0];
  dy = x1[1] - x2[1];

  ri   = 1.0f/q1[0];
  p1   = gm1*(q1[3]-0.5f*ri*(q1[1]*q1[1]+q1[2]*q1[2]));
  vol1 =  ri*(q1[1]*dy - q1[2]*dx);

  ri   = 1.0f/q2[0];
  p2   = gm1*(q2[3]-0.5f*ri*(q2[1]*q2[1]+q2[2]*q2[2]));
  vol2 =  ri*(q2[1]*dy - q2[2]*dx);

  mu = 0.5f*((*adt1)+(*adt2))*eps;

  f = 0.5f*(vol1* q1[0]         + vol2* q2[0]        ) + mu*(q1[0]-q2[0]);
  res1[0] += f;
  res2[0] -= f;
  f = 0.5f*(vol1* q1[1] + p1*dy + vol2* q2[1] + p2*dy) + mu*(q1[1]-q2[1]);
  res1[1] += f;
  res2[1] -= f;
  f = 0.5f*(vol1* q1[2] - p1*dx + vol2* q2[2] - p2*dx) + mu*(q1[2]-q2[2]);
  res1[2] += f;
  res2[2] -= f;
  f = 0.5f*(vol1*(q1[3]+p1)     + vol2*(q2[3]+p2)    ) + mu*(q1[3]-q2[3]);
  res1[3] += f;
  res2[3] -= f;
}


#endif

// host stub function
void op_par_loop_res_calc(char const *name, op_set set,
  op_arg arg0,
  op_arg arg1,
  op_arg arg2,
  op_arg arg3,
  op_arg arg4,
  op_arg arg5,
  op_arg arg6,
  op_arg arg7){

  int nargs = 8;
  op_arg args[8];

  args[0] = arg0;
  args[1] = arg1;
  args[2] = arg2;
  args[3] = arg3;
  args[4] = arg4;
  args[5] = arg5;
  args[6] = arg6;
  args[7] = arg7;
  int  ninds   = 4;
  int  inds[8] = {0,0,1,1,2,2,3,3};

  #ifdef OP_PART_SIZE_2
    int part_size = OP_PART_SIZE_2;
  #else
    int part_size = OP_part_size;
  #endif

  // set number of threads
  #ifdef _OPENMP
    int nthreads = omp_get_max_threads();
  #else
    int nthreads = 1;
  #endif

  // initialise timers
  double cpu_t1, cpu_t2, wall_t1, wall_t2;
  op_timing_realloc(2);
  op_timers_core(&cpu_t1, &wall_t1);


  int exec_size = op_mpi_halo_exchanges(set, nargs, args);
  int set_size = ((set->size+set->exec_size-1)/16+1)*16; //align to 512 bits
  op_plan *Plan = op_plan_get_stage(name,set,part_size,nargs,args,ninds,inds, OP_STAGE_PERMUTE);
  if (OP_diags>2) {
    printf(" kernel routine with indirection: res_calc\n");
  }

  if (exec_size >0) {

    int block_offset = 0;
    for ( int col=0; col<Plan->ncolors; col++ ){
      if (col==Plan->ncolors_core) {
        op_mpi_wait_all(nargs, args);
      }
      int nblocks = Plan->ncolblk[col];

      #pragma omp parallel for
      for ( int blockIdx=0; blockIdx<nblocks; blockIdx++ ){
        int blockId  = Plan->blkmap[blockIdx + block_offset];
        int nelem    = Plan->nelems[blockId];
        int offset_b = Plan->offset[blockId];
        for ( int col2=0; col2<Plan->nthrcol[blockId]; col2++ ){
          int start = offset_b + Plan->col_offsets[blockId][col2];
          int end = offset_b + Plan->col_offsets[blockId][col2+1];
#ifdef VECTORIZE
          //assumes blocksize is multiple of vector size
          int presweep = min(((start-1)/VECSIZEH+1)*VECSIZEH,end);
          for ( int e=start; e<presweep; e++ ){
            int n = offset_b + Plan->col_reord[e];
            int map0idx = arg0.map_data[n * arg0.map->dim + 0];
            int map1idx = arg0.map_data[n * arg0.map->dim + 1];
            int map2idx = arg2.map_data[n * arg2.map->dim + 0];
            int map3idx = arg2.map_data[n * arg2.map->dim + 1];

            res_calc(
              &((double*)arg0.data)[2 * map0idx],
              &((double*)arg0.data)[2 * map1idx],
              &((double*)arg2.data)[4 * map2idx],
              &((double*)arg2.data)[4 * map3idx],
              &((double*)arg4.data)[1 * map2idx],
              &((double*)arg4.data)[1 * map3idx],
              &((double*)arg6.data)[4 * map2idx],
              &((double*)arg6.data)[4 * map3idx]);
          }
          for ( int e=presweep/VECSIZEH; e<end/VECSIZEH; e++ ){
            intv_half n(&Plan->col_reord[VECSIZEH*e ]);
            n += offset_b;
            intv_half map0idx(&arg0.map_data_d[set_size * 0], n);
            intv_half map1idx(&arg0.map_data_d[set_size * 1], n);
            intv_half map2idx(&arg2.map_data_d[set_size * 0], n);
            intv_half map3idx(&arg2.map_data_d[set_size * 1], n);

            intv_half mapidx;
            mapidx = 2*map0idx;
            doublev arg0_p[2] = {
              doublev((double*)arg0.data+0, mapidx),
              doublev((double*)arg0.data+1, mapidx)};
            mapidx = 2*map1idx;
            doublev arg1_p[2] = {
              doublev((double*)arg1.data+0, mapidx),
              doublev((double*)arg1.data+1, mapidx)};
            mapidx = 4*map2idx;
            doublev arg2_p[4] = {
              doublev((double*)arg2.data+0, mapidx),
              doublev((double*)arg2.data+1, mapidx),
              doublev((double*)arg2.data+2, mapidx),
              doublev((double*)arg2.data+3, mapidx)};
            mapidx = 4*map3idx;
            doublev arg3_p[4] = {
              doublev((double*)arg3.data+0, mapidx),
              doublev((double*)arg3.data+1, mapidx),
              doublev((double*)arg3.data+2, mapidx),
              doublev((double*)arg3.data+3, mapidx)};
            doublev arg4_p[1] = {
              doublev((double*)arg4.data+0, map2idx)};
            doublev arg5_p[1] = {
              doublev((double*)arg5.data+0, map3idx)};
            doublev arg6_p[4] = {
              doublev(0.0),
              doublev(0.0),
              doublev(0.0),
              doublev(0.0)};
            doublev arg7_p[4] = {
              doublev(0.0),
              doublev(0.0),
              doublev(0.0),
              doublev(0.0)};
            res_calc_vec(
              arg0_p,
              arg1_p,
              arg2_p,
              arg3_p,
              arg4_p,
              arg5_p,
              arg6_p,
              arg7_p);
            mapidx = 4*map2idx;
            store_scatter_add_safe(arg6_p[0], (double*)arg6.data+0, mapidx);
            store_scatter_add_safe(arg6_p[1], (double*)arg6.data+1, mapidx);
            store_scatter_add_safe(arg6_p[2], (double*)arg6.data+2, mapidx);
            store_scatter_add_safe(arg6_p[3], (double*)arg6.data+3, mapidx);
            mapidx = 4*map3idx;
            store_scatter_add_safe(arg7_p[0], (double*)arg7.data+0, mapidx);
            store_scatter_add_safe(arg7_p[1], (double*)arg7.data+1, mapidx);
            store_scatter_add_safe(arg7_p[2], (double*)arg7.data+2, mapidx);
            store_scatter_add_safe(arg7_p[3], (double*)arg7.data+3, mapidx);
          }
          int postsweep = max((end/VECSIZEH)*VECSIZEH, presweep);
          for ( int e=postsweep; e<end; e++ ){
#else
          #pragma ivdep
          for ( int e=start; e<end; e++ ){
#endif
            int n = offset_b + Plan->col_reord[e];
            int map0idx = arg0.map_data[n * arg0.map->dim + 0];
            int map1idx = arg0.map_data[n * arg0.map->dim + 1];
            int map2idx = arg2.map_data[n * arg2.map->dim + 0];
            int map3idx = arg2.map_data[n * arg2.map->dim + 1];

            res_calc(
              &((double*)arg0.data)[2 * map0idx],
              &((double*)arg0.data)[2 * map1idx],
              &((double*)arg2.data)[4 * map2idx],
              &((double*)arg2.data)[4 * map3idx],
              &((double*)arg4.data)[1 * map2idx],
              &((double*)arg4.data)[1 * map3idx],
              &((double*)arg6.data)[4 * map2idx],
              &((double*)arg6.data)[4 * map3idx]);
          }
        }
      }
      block_offset += nblocks;
    }
    OP_kernels[2].transfer  += Plan->transfer;
    OP_kernels[2].transfer2 += Plan->transfer2;
  }

  if (exec_size == 0 || exec_size == set->core_size) {
    op_mpi_wait_all(nargs, args);
  }
  op_mpi_set_dirtybit(nargs, args);

  // update kernel record
  op_timers_core(&cpu_t2, &wall_t2);
  OP_kernels[2].name      = name;
  OP_kernels[2].count    += 1;
  OP_kernels[2].time     += wall_t2 - wall_t1;
}
