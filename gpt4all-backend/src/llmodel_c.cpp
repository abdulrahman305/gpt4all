#include "llmodel_c.h"

#include "llmodel.h"

#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <exception>
#include <iostream>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <vector>
#include <span>

namespace ranges = std::ranges;

static_assert(sizeof(token_t) == sizeof(LLModel::Token));
#include <cstring>
#include <cerrno>
#include <utility>

struct LLModelWrapper {
    LLModel *llModel = nullptr;
    ~LLModelWrapper() { delete llModel; }
};

llmodel_model llmodel_model_create(const char *model_path)
{
    const char *error;
    auto fres = llmodel_model_create2(model_path, "auto", &error);

thread_local static std::string last_error_message;


llmodel_model llmodel_model_create(const char *model_path) {
    auto fres = llmodel_model_create2(model_path, "auto", nullptr);
    if (!fres) {
        fprintf(stderr, "Invalid model file\n");
    }
    return fres;
}

static void llmodel_set_error(const char **errptr, const char *message)
{
    thread_local static std::string last_error_message;
    if (errptr) {
        last_error_message = message;
        *errptr = last_error_message.c_str();
    }
}

llmodel_model llmodel_model_create2(const char *model_path, const char *backend, const char **error)
{
    LLModel *llModel;
    try {
        llModel = LLModel::Implementation::construct(model_path, backend);
    } catch (const std::exception& e) {
        llmodel_set_error(error, e.what());
        return nullptr;
    }

llmodel_model llmodel_model_create2(const char *model_path, const char *build_variant, llmodel_error *error) {
    auto wrapper = new LLModelWrapper;
    int error_code = 0;

    try {
        wrapper->llModel = LLModel::Implementation::construct(model_path, build_variant);
    } catch (const std::exception& e) {
        error_code = EINVAL;
        last_error_message = e.what();
    }

    if (!wrapper->llModel) {
        delete std::exchange(wrapper, nullptr);
        // Get errno and error message if none
        if (error_code == 0) {
            if (errno != 0) {
                error_code = errno;
                last_error_message = std::strerror(error_code);
            } else {
                error_code = ENOTSUP;
                last_error_message = "Model format not supported (no matching implementation found)";
            }
        }
        // Set error argument
        if (error) {
            error->message = last_error_message.c_str();
            error->code = error_code;
        }
    }
    return reinterpret_cast<llmodel_model*>(wrapper);
}

void llmodel_model_destroy(llmodel_model model)
{
    delete static_cast<LLModelWrapper *>(model);
void llmodel_model_destroy(llmodel_model model) {
    delete reinterpret_cast<LLModelWrapper*>(model);
}

size_t llmodel_required_mem(llmodel_model model, const char *model_path)
{
    LLModelWrapper *wrapper = reinterpret_cast<LLModelWrapper*>(model);
    return wrapper->llModel->requiredMem(model_path);
}

bool llmodel_loadModel(llmodel_model model, const char *model_path)
{
    LLModelWrapper *wrapper = reinterpret_cast<LLModelWrapper*>(model);
    return wrapper->llModel->loadModel(model_path);
}

bool llmodel_isModelLoaded(llmodel_model model)
{
    LLModelWrapper *wrapper = reinterpret_cast<LLModelWrapper*>(model);
    return wrapper->llModel->isModelLoaded();
}

uint64_t llmodel_state_get_size(llmodel_model model)
{
    LLModelWrapper *wrapper = reinterpret_cast<LLModelWrapper*>(model);
    return wrapper->llModel->stateSize();
}

uint64_t llmodel_state_get_data(llmodel_model model, uint8_t *state_out, uint64_t state_size,
                                token_t **input_tokens_out, uint64_t *n_input_tokens)
{
    auto *wrapper = static_cast<LLModelWrapper *>(model);
    std::vector<LLModel::Token> inputTokens;
    auto bytesWritten = wrapper->llModel->saveState({state_out, size_t(state_size)}, inputTokens);
    if (bytesWritten) {
        auto *buf = new LLModel::Token[inputTokens.size()];
        ranges::copy(inputTokens, buf);
        *input_tokens_out = buf;
        *n_input_tokens = uint64_t(inputTokens.size());
    } else {
        *input_tokens_out = nullptr;
        *n_input_tokens = 0;
    }
    return bytesWritten;
    LLModelWrapper *wrapper = reinterpret_cast<LLModelWrapper*>(model);
    return wrapper->llModel->saveState(dest);
}

void llmodel_state_free_input_tokens(LLModel::Token *input_tokens)
{
    delete[] input_tokens;
}

uint64_t llmodel_state_set_data(llmodel_model model, const uint8_t *state, uint64_t state_size,
                                const token_t *input_tokens, uint64_t n_input_tokens)
{
    auto *wrapper = static_cast<LLModelWrapper *>(model);
    return wrapper->llModel->restoreState({state, size_t(state_size)}, {input_tokens, size_t(n_input_tokens)});
}

bool llmodel_prompt(llmodel_model               model,
                    const char                 *prompt,
                    llmodel_prompt_callback     prompt_callback,
                    llmodel_response_callback   response_callback,
                    llmodel_prompt_context     *ctx,
                    const char                **error)
{
    auto *wrapper = static_cast<LLModelWrapper *>(model);

    // Copy the C prompt context
    LLModel::PromptContext promptContext {
        .n_predict      = ctx->n_predict,
        .top_k          = ctx->top_k,
        .top_p          = ctx->top_p,
        .min_p          = ctx->min_p,
        .temp           = ctx->temp,
        .n_batch        = ctx->n_batch,
        .repeat_penalty = ctx->repeat_penalty,
        .repeat_last_n  = ctx->repeat_last_n,
        .contextErase   = ctx->context_erase,
    };

    auto prompt_func = [prompt_callback](std::span<const LLModel::Token> token_ids, bool cached) {
        return prompt_callback(token_ids.data(), token_ids.size(), cached);
    };
    auto response_func = [response_callback](LLModel::Token token_id, std::string_view piece) {
        return response_callback(token_id, piece.data());
    };

    // Call the C++ prompt method
    try {
        wrapper->llModel->prompt(prompt, prompt_func, response_func, promptContext);
    } catch (std::exception const &e) {
        llmodel_set_error(error, e.what());
        return false;
    }

    return true;
}

float *llmodel_embed(
    llmodel_model model, const char **texts, size_t *embedding_size, const char *prefix, int dimensionality,
    size_t *token_count, bool do_mean, bool atlas, llmodel_emb_cancel_callback cancel_cb, const char **error
) {
    auto *wrapper = static_cast<LLModelWrapper *>(model);

    if (!texts || !*texts) {
        llmodel_set_error(error, "'texts' is NULL or empty");
        return nullptr;
    }

    std::vector<std::string> textsVec;
    while (*texts) { textsVec.emplace_back(*texts++); }

    size_t embd_size;
    float *embedding;

    try {
        embd_size = wrapper->llModel->embeddingSize();
        if (dimensionality > 0 && dimensionality < int(embd_size))
            embd_size = dimensionality;

        embd_size *= textsVec.size();

        std::optional<std::string> prefixStr;
        if (prefix) { prefixStr = prefix; }

        embedding = new float[embd_size];
        wrapper->llModel->embed(textsVec, embedding, prefixStr, dimensionality, token_count, do_mean, atlas, cancel_cb);
    } catch (std::exception const &e) {
        llmodel_set_error(error, e.what());
    LLModelWrapper *wrapper = reinterpret_cast<LLModelWrapper*>(model);
    return wrapper->llModel->restoreState(src);
}

// Wrapper functions for the C callbacks
bool prompt_wrapper(int32_t token_id, void *user_data) {
    llmodel_prompt_callback callback = reinterpret_cast<llmodel_prompt_callback>(user_data);
    return callback(token_id);
}

bool response_wrapper(int32_t token_id, const std::string &response, void *user_data) {
    llmodel_response_callback callback = reinterpret_cast<llmodel_response_callback>(user_data);
    return callback(token_id, response.c_str());
}

bool recalculate_wrapper(bool is_recalculating, void *user_data) {
    llmodel_recalculate_callback callback = reinterpret_cast<llmodel_recalculate_callback>(user_data);
    return callback(is_recalculating);
}

void llmodel_prompt(llmodel_model model, const char *prompt,
                    llmodel_prompt_callback prompt_callback,
                    llmodel_response_callback response_callback,
                    llmodel_recalculate_callback recalculate_callback,
                    llmodel_prompt_context *ctx)
{
    LLModelWrapper *wrapper = reinterpret_cast<LLModelWrapper*>(model);

    // Create std::function wrappers that call the C function pointers
    std::function<bool(int32_t)> prompt_func =
        std::bind(&prompt_wrapper, std::placeholders::_1, reinterpret_cast<void*>(prompt_callback));
    std::function<bool(int32_t, const std::string&)> response_func =
        std::bind(&response_wrapper, std::placeholders::_1, std::placeholders::_2, reinterpret_cast<void*>(response_callback));
    std::function<bool(bool)> recalc_func =
        std::bind(&recalculate_wrapper, std::placeholders::_1, reinterpret_cast<void*>(recalculate_callback));

    if (size_t(ctx->n_past) < wrapper->promptContext.tokens.size())
        wrapper->promptContext.tokens.resize(ctx->n_past);

    // Copy the C prompt context
    wrapper->promptContext.n_past = ctx->n_past;
    wrapper->promptContext.n_ctx = ctx->n_ctx;
    wrapper->promptContext.n_predict = ctx->n_predict;
    wrapper->promptContext.top_k = ctx->top_k;
    wrapper->promptContext.top_p = ctx->top_p;
    wrapper->promptContext.temp = ctx->temp;
    wrapper->promptContext.n_batch = ctx->n_batch;
    wrapper->promptContext.repeat_penalty = ctx->repeat_penalty;
    wrapper->promptContext.repeat_last_n = ctx->repeat_last_n;
    wrapper->promptContext.contextErase = ctx->context_erase;

    // Call the C++ prompt method
    wrapper->llModel->prompt(prompt, prompt_func, response_func, recalc_func, wrapper->promptContext);

    // Update the C context by giving access to the wrappers raw pointers to std::vector data
    // which involves no copies
    ctx->logits = wrapper->promptContext.logits.data();
    ctx->logits_size = wrapper->promptContext.logits.size();
    ctx->tokens = wrapper->promptContext.tokens.data();
    ctx->tokens_size = wrapper->promptContext.tokens.size();

    // Update the rest of the C prompt context
    ctx->n_past = wrapper->promptContext.n_past;
    ctx->n_ctx = wrapper->promptContext.n_ctx;
    ctx->n_predict = wrapper->promptContext.n_predict;
    ctx->top_k = wrapper->promptContext.top_k;
    ctx->top_p = wrapper->promptContext.top_p;
    ctx->temp = wrapper->promptContext.temp;
    ctx->n_batch = wrapper->promptContext.n_batch;
    ctx->repeat_penalty = wrapper->promptContext.repeat_penalty;
    ctx->repeat_last_n = wrapper->promptContext.repeat_last_n;
    ctx->context_erase = wrapper->promptContext.contextErase;
}

float *llmodel_embedding(llmodel_model model, const char *text, size_t *embedding_size)
{
    if (model == nullptr || text == nullptr || !strlen(text)) {
        *embedding_size = 0;
        return nullptr;
    }
    LLModelWrapper *wrapper = reinterpret_cast<LLModelWrapper*>(model);
    std::vector<float> embeddingVector = wrapper->llModel->embedding(text);
    float *embedding = (float *)malloc(embeddingVector.size() * sizeof(float));
    if (embedding == nullptr) {
        *embedding_size = 0;
        return nullptr;
    }
    std::copy(embeddingVector.begin(), embeddingVector.end(), embedding);
    *embedding_size = embeddingVector.size();
    return embedding;
}

void llmodel_free_embedding(float *ptr)
{
    free(ptr);
}

void llmodel_setThreadCount(llmodel_model model, int32_t n_threads)
{
    LLModelWrapper *wrapper = reinterpret_cast<LLModelWrapper*>(model);
    wrapper->llModel->setThreadCount(n_threads);
}

int32_t llmodel_threadCount(llmodel_model model)
{
    LLModelWrapper *wrapper = reinterpret_cast<LLModelWrapper*>(model);
    return wrapper->llModel->threadCount();
}

void llmodel_set_implementation_search_path(const char *path)
{
    LLModel::Implementation::setImplementationsSearchPath(path);
}

const char *llmodel_get_implementation_search_path()
{
    return LLModel::Implementation::implementationsSearchPath().c_str();
}

struct llmodel_gpu_device* llmodel_available_gpu_devices(llmodel_model model, size_t memoryRequired, int* num_devices)
{
    LLModelWrapper *wrapper = reinterpret_cast<LLModelWrapper*>(model);
    std::vector<LLModel::GPUDevice> devices = wrapper->llModel->availableGPUDevices(memoryRequired);

    // Set the num_devices
    *num_devices = devices.size();

    if (*num_devices == 0) return nullptr;  // Return nullptr if no devices are found

    c_devices = std::make_unique<llmodel_gpu_device_cpp[]>(devices.size());
    for (unsigned i = 0; i < devices.size(); i++) {
        const auto &dev  =   devices[i];
              auto &cdev = c_devices[i];
        cdev.backend  = dev.backend;
        cdev.index    = dev.index;
        cdev.type     = dev.type;
        cdev.heapSize = dev.heapSize;
        cdev.name     = strdup(dev.name.c_str());
        cdev.vendor   = strdup(dev.vendor.c_str());
    // Allocate memory for the output array
    struct llmodel_gpu_device* output = (struct llmodel_gpu_device*) malloc(*num_devices * sizeof(struct llmodel_gpu_device));

    for (int i = 0; i < *num_devices; i++) {
        output[i].index = devices[i].index;
        output[i].type = devices[i].type;
        output[i].heapSize = devices[i].heapSize;
        output[i].name = strdup(devices[i].name.c_str());  // Convert std::string to char* and allocate memory
        output[i].vendor = strdup(devices[i].vendor.c_str());  // Convert std::string to char* and allocate memory
    }

    return output;
}

bool llmodel_gpu_init_gpu_device_by_string(llmodel_model model, size_t memoryRequired, const char *device)
{
    LLModelWrapper *wrapper = reinterpret_cast<LLModelWrapper*>(model);
    return wrapper->llModel->initializeGPUDevice(memoryRequired, std::string(device));
}

bool llmodel_gpu_init_gpu_device_by_struct(llmodel_model model, const llmodel_gpu_device *device)
{
    LLModel::GPUDevice d;
    d.index = device->index;
    d.type = device->type;
    d.heapSize = device->heapSize;
    d.name = device->name;
    d.vendor = device->vendor;
    LLModelWrapper *wrapper = reinterpret_cast<LLModelWrapper*>(model);
    return wrapper->llModel->initializeGPUDevice(d);
}

bool llmodel_gpu_init_gpu_device_by_int(llmodel_model model, int device)
{
    LLModelWrapper *wrapper = reinterpret_cast<LLModelWrapper*>(model);
    return wrapper->llModel->initializeGPUDevice(device);
}

const char *llmodel_model_backend_name(llmodel_model model)
{
    const auto *wrapper = static_cast<LLModelWrapper *>(model);
    return wrapper->llModel->backendName();
}

const char *llmodel_model_gpu_device_name(llmodel_model model)
{
    const auto *wrapper = static_cast<LLModelWrapper *>(model);
    return wrapper->llModel->gpuDeviceName();
}

int32_t llmodel_count_prompt_tokens(llmodel_model model, const char *prompt, const char **error)
{
    auto *wrapper = static_cast<const LLModelWrapper *>(model);
    try {
        return wrapper->llModel->countPromptTokens(prompt);
    } catch (const std::exception& e) {
        llmodel_set_error(error, e.what());
        return -1;
    }
}

void llmodel_model_foreach_special_token(llmodel_model model, llmodel_special_token_callback callback)
{
    auto *wrapper = static_cast<const LLModelWrapper *>(model);
    for (auto &[name, token] : wrapper->llModel->specialTokens())
        callback(name.c_str(), token.c_str());
    LLModelWrapper *wrapper = reinterpret_cast<LLModelWrapper*>(model);
    return wrapper->llModel->hasGPUDevice();
}
