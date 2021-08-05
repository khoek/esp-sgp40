
#include "device/sgp40.h"
#include "sensirion_voc_algorithm.h"

struct sgp40_voc_algorithm_data {
    VocAlgorithmParams params;
};

void sgp40_voc_algorithm_ctx_init(sgp40_voc_algorithm_ctx_t* out_ctx) {
    sgp40_voc_algorithm_data_t* ctx =
        malloc(sizeof(sgp40_voc_algorithm_data_t));
    VocAlgorithm_init(&ctx->params);

    *out_ctx = ctx;
}

void sgp40_voc_algorithm_ctx_destroy(sgp40_voc_algorithm_ctx_t ctx) {
    free(ctx);
}

void sgp40_voc_algorithm_ctx_process(sgp40_voc_algorithm_ctx_t ctx,
                                     uint16_t sraw, int32_t* voc_index) {
    VocAlgorithm_process(&ctx->params, sraw, voc_index);
}
