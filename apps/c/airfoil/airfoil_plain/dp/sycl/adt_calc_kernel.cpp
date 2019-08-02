//
// auto-generated by op2.py
//

//user function
class adt_calc_kernel;

//host stub function
void op_par_loop_adt_calc(char const *name, op_set set,
  op_arg arg0,
  op_arg arg1,
  op_arg arg2,
  op_arg arg3,
  op_arg arg4,
  op_arg arg5){

  int nargs = 6;
  op_arg args[6];

  args[0] = arg0;
  args[1] = arg1;
  args[2] = arg2;
  args[3] = arg3;
  args[4] = arg4;
  args[5] = arg5;

  // initialise timers
  double cpu_t1, cpu_t2, wall_t1, wall_t2;
  op_timing_realloc(1);
  op_timers_core(&cpu_t1, &wall_t1);
  OP_kernels[1].name      = name;
  OP_kernels[1].count    += 1;


  int    ninds   = 1;
  int    inds[6] = {0,0,0,0,-1,-1};

  if (OP_diags>2) {
    printf(" kernel routine with indirection: adt_calc\n");
  }

  //get plan
  #ifdef OP_PART_SIZE_1
    int part_size = OP_PART_SIZE_1;
  #else
    int part_size = OP_part_size;
  #endif

  op_mpi_halo_exchanges_cuda(set, nargs, args);
  if (set->size > 0) {

    op_plan *Plan = op_plan_get_stage(name,set,part_size,nargs,args,ninds,inds,OP_COLOR2);

    cl::sycl::buffer<double,1> *arg0_buffer = static_cast<cl::sycl::buffer<double,1>*>((void*)arg0.data_d);
    cl::sycl::buffer<int,1> *map0_buffer = static_cast<cl::sycl::buffer<int,1>*>((void*)arg0.map_data_d);
    cl::sycl::buffer<double,1> *arg4_buffer = static_cast<cl::sycl::buffer<double,1>*>((void*)arg4.data_d);
    cl::sycl::buffer<double,1> *arg5_buffer = static_cast<cl::sycl::buffer<double,1>*>((void*)arg5.data_d);
    cl::sycl::buffer<int,1> *col_reord_buffer = static_cast<cl::sycl::buffer<int,1>*>((void*)Plan->col_reord);
    int set_size = set->size+set->exec_size;
    //execute plan
    for ( int col=0; col<Plan->ncolors; col++ ){
      if (col==Plan->ncolors_core) {
        op_mpi_wait_all_cuda(nargs, args);
      }
      #ifdef OP_BLOCK_SIZE_1
      int nthread = OP_BLOCK_SIZE_1;
      #else
      int nthread = OP_block_size;
      #endif

      int start = Plan->col_offsets[0][col];
      int end = Plan->col_offsets[0][col+1];
      int nblocks = (end - start - 1)/nthread + 1;
      try {
      op2_queue->submit([&](cl::sycl::handler& cgh) {
        auto ind_arg0 = (*arg0_buffer).template get_access<cl::sycl::access::mode::read_write>(cgh);
        auto opDat0Map =  (*map0_buffer).template get_access<cl::sycl::access::mode::read>(cgh);
        auto col_reord = (*col_reord_buffer).template get_access<cl::sycl::access::mode::read>(cgh);

        auto arg4 = (*arg4_buffer).template get_access<cl::sycl::access::mode::read_write>(cgh);
        auto arg5 = (*arg5_buffer).template get_access<cl::sycl::access::mode::read_write>(cgh);
        auto gam_sycl = (*gam_p).template get_access<cl::sycl::access::mode::read>(cgh);
        auto gm1_sycl = (*gm1_p).template get_access<cl::sycl::access::mode::read>(cgh);
        auto cfl_sycl = (*cfl_p).template get_access<cl::sycl::access::mode::read>(cgh);

        //user fun as lambda
        auto adt_calc_gpu = [=]( const double *x1, const double *x2, const double *x3,
                               const double *x4, const double *q, double *adt) {
            double dx, dy, ri, u, v, c;
          
            ri = 1.0f / q[0];
            u = ri * q[1];
            v = ri * q[2];
            c = cl::sycl::sqrt(gam_sycl[0] * gm1_sycl[0] * (ri * q[3] - 0.5f * (u * u + v * v)));
          
            dx = x2[0] - x1[0];
            dy = x2[1] - x1[1];
            *adt = fabs(u * dy - v * dx) + c * cl::sycl::sqrt(dx * dx + dy * dy);
          
            dx = x3[0] - x2[0];
            dy = x3[1] - x2[1];
            *adt += fabs(u * dy - v * dx) + c * cl::sycl::sqrt(dx * dx + dy * dy);
          
            dx = x4[0] - x3[0];
            dy = x4[1] - x3[1];
            *adt += fabs(u * dy - v * dx) + c * cl::sycl::sqrt(dx * dx + dy * dy);
          
            dx = x1[0] - x4[0];
            dy = x1[1] - x4[1];
            *adt += fabs(u * dy - v * dx) + c * cl::sycl::sqrt(dx * dx + dy * dy);
          
            *adt = (*adt) / cfl_sycl[0];
          
          };
          
        auto kern = [=](cl::sycl::nd_item<1> item) {
          int tid = item.get_global_linear_id();
          if (tid + start < end) {
            int n = col_reord[tid + start];
            //initialise local variables
            int map0idx;
            int map1idx;
            int map2idx;
            int map3idx;
            map0idx = opDat0Map[n + set_size * 0];
            map1idx = opDat0Map[n + set_size * 1];
            map2idx = opDat0Map[n + set_size * 2];
            map3idx = opDat0Map[n + set_size * 3];

            //user-supplied kernel call
            adt_calc_gpu(&ind_arg0[map0idx*2],
             &ind_arg0[map1idx*2],
             &ind_arg0[map2idx*2],
             &ind_arg0[map3idx*2],
             &arg4[n*4],
             &arg5[n*1]);
          }

        };
        cgh.parallel_for<class adt_calc_kernel>(cl::sycl::nd_range<1>(nthread*nblocks,nthread), kern);
      });
      }catch(cl::sycl::exception const &e) {
      std::cout << e.what() << std::endl;exit(-1);
      }

    }
    OP_kernels[1].transfer  += Plan->transfer;
    OP_kernels[1].transfer2 += Plan->transfer2;
  }
  op_mpi_set_dirtybit_cuda(nargs, args);
  op2_queue->wait();
  //update kernel record
  op_timers_core(&cpu_t2, &wall_t2);
  OP_kernels[1].time     += wall_t2 - wall_t1;
}