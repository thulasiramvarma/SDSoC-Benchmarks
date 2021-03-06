
#include "spmv.h"
#include <hls_stream.h>


void spvm_kernel(
		DATA_TYPE  *values,
		u32        *cols,
		u32        *rows,
		DATA_TYPE  *x_local,
		DATA_TYPE  *y,
		u32         row_size,
		u32         col_size,
		u32         data_size
) {

#pragma HLS DATAFLOW

	hls::stream<u32>       rows_fifo;
	hls::stream<DATA_TYPE> values_fifo;
	hls::stream<u32>       cols_fifo;
	hls::stream<DATA_TYPE> results_fifo;
	hls::stream<DATA_TYPE> results_fifo_middle;
	hls::stream<DATA_TYPE> data_length_fifo;
	hls::stream<DATA_TYPE> row_index_fifo;



	u32 i, r;
	for (i = 0; i < row_size; i++) {
#pragma HLS PIPELINE
		rows_fifo << *(rows+i);
	}

	for (i = 0; i < data_size; i++) {
#pragma HLS PIPELINE
		values_fifo << *(values+i);
		u32 c =*(cols+i);
		cols_fifo   << c;
	}


	u32 col_left=0;
	

	DATA_TYPE sum;
	DATA_TYPE value;
	u32 col;

	//u32 row_index = 0;

	for (r = 0; r < data_size; r++) {
#pragma HLS PIPELINE
		if (col_left == 0) {
			col_left = rows_fifo.read();
			sum = 0;
		}
		value = values_fifo.read();
		col   = cols_fifo.read();
		sum  += value * x_local[col];

		col_left--;

		if (col_left == 0) {
			results_fifo << sum;
		}
	}


	for (i = 0; i < row_size; i++) {
#pragma HLS PIPELINE
		*(y+i) = results_fifo.read();
	}

}



#pragma SDS data zero_copy(values[0:DATA_LENGTH])
#pragma SDS data mem_attribute(values:PHYSICAL_CONTIGUOUS)
#pragma SDS data zero_copy(cols[0:DATA_LENGTH])
#pragma SDS data mem_attribute(cols:PHYSICAL_CONTIGUOUS)
#pragma SDS data zero_copy(rows[0:ROWS])
#pragma SDS data mem_attribute(rows:PHYSICAL_CONTIGUOUS)
#pragma SDS data zero_copy(x[0:COLS])
#pragma SDS data mem_attribute(x:PHYSICAL_CONTIGUOUS)
#pragma SDS data zero_copy(y[0:ROWS])
#pragma SDS data mem_attribute(y:PHYSICAL_CONTIGUOUS)
int spmv_accel(
		DATA_TYPE       values[DATA_LENGTH],
		u32        		cols[DATA_LENGTH],
		u32        		rows[ROWS],
		DATA_TYPE  		x[COLS],
		DATA_TYPE  		y[ROWS],

		u32        		 row_size,
		u32        		 col_size,
		u32        		 data_size
	) {

	DATA_TYPE              x_local[MAX_COL_SIZE];

	for (u32 i = 0; i < col_size; i++) {
#pragma HLS PIPELINE
		x_local[i] = *(x+i);
	}


	spvm_kernel(values, cols, rows, x_local, y, row_size, col_size, data_size);

	return 0;
}
