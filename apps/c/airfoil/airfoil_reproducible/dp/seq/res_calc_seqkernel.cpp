//
// auto-generated by op2.py
//
#include <mpi.h>
//user function
#include "../res_calc.h"

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

  // initialise timers
  double cpu_t1, cpu_t2, wall_t1, wall_t2;
  op_timing_realloc(2);
  op_timers_core(&cpu_t1, &wall_t1);

  if (OP_diags>2) {
    printf(" kernel routine with indirection: res_calc\n");
  }

  int set_size = op_mpi_halo_exchanges(set, nargs, args);

  if (set->size >0) {

    op_map prime_map = arg6.map; //TODO works only with arg6...
    op_reversed_map rev_map = OP_reversed_map_list[prime_map->index];

    if (rev_map != NULL) {
        int prime_map_dim = prime_map->dim;
        int set_from_size = prime_map->from->size + prime_map->from->exec_size ;
        int set_to_size = prime_map->to->size + prime_map->to->exec_size + prime_map->to->nonexec_size;

        int required_tmp_incs_size = set_from_size * prime_map_dim * arg6.dat->size;
    
        if (op_repr_incs[arg6.dat->index].tmp_incs == NULL){
            op_repr_incs[arg6.dat->index].tmp_incs = (void *)op_malloc(required_tmp_incs_size);
            op_repr_incs[arg6.dat->index].tmp_incs_size = required_tmp_incs_size;
        } else if (op_repr_incs[arg6.dat->index].tmp_incs_size < required_tmp_incs_size){
            op_realloc(op_repr_incs[arg6.dat->index].tmp_incs, required_tmp_incs_size);
            op_repr_incs[arg6.dat->index].tmp_incs_size = required_tmp_incs_size;
        }

        double *tmp_incs = (double *)op_repr_incs[arg6.dat->index].tmp_incs;

        for (int i=0; i<set_from_size * prime_map_dim * arg6.dim; i++){
          tmp_incs[i]=0.0;
        }

        for ( int n=0; n<set_size; n++ ){

          if (n==set->core_size) {
            op_mpi_wait_all(nargs, args);
          }
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
              &tmp_incs[(n*prime_map_dim+0)*arg6.dim],
              &tmp_incs[(n*prime_map_dim+1)*arg6.dim]);
        }

        for ( int n=0; n<set_to_size; n++ ){
            for ( int i=0; i<rev_map->row_start_idx[n+1] - rev_map->row_start_idx[n]; i++){
                for (int d=0; d<arg6.dim; d++){
                    ((double*)arg6.data)[arg6.dim * n + d] +=
                    tmp_incs[rev_map->reversed_map[rev_map->row_start_idx[n]+i] * arg6.dim + d];
                }
            }
        }

    }
  }


  if (set_size == 0 || set_size == set->core_size) {
    op_mpi_wait_all(nargs, args);
  }
  // combine reduction data
  op_mpi_set_dirtybit(nargs, args);

  // update kernel record
  op_timers_core(&cpu_t2, &wall_t2);
  OP_kernels[2].name      = name;
  OP_kernels[2].count    += 1;
  OP_kernels[2].time     += wall_t2 - wall_t1;
  OP_kernels[2].transfer += (float)set->size * arg0.size;
  OP_kernels[2].transfer += (float)set->size * arg2.size;
  OP_kernels[2].transfer += (float)set->size * arg4.size;
  OP_kernels[2].transfer += (float)set->size * arg6.size * 2.0f;
  OP_kernels[2].transfer += (float)set->size * arg0.map->dim * 4.0f;
  OP_kernels[2].transfer += (float)set->size * arg2.map->dim * 4.0f;
}
