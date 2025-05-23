# GPT4ALL Backend
This directory contains the C/C++ model backend used by GPT4All for inference on the CPU. This backend acts as a universal library/wrapper for all models that the GPT4All ecosystem supports. Language bindings are built on top of this universal library. The native GPT4all Chat application directly uses this library for all inference.

# What models are supported by the GPT4All ecosystem?

Currently, there are multiple different model architectures that are supported:

1. GPTJ - Based off of the GPT-J architecture with examples found [here](https://huggingface.co/EleutherAI/gpt-j-6b)
2. LLAMA - Based off of the LLAMA architecture with examples found [here](https://huggingface.co/models?sort=downloads&search=llama)
3. MPT - Based off of Mosaic ML's MPT architecture with examples found [here](https://huggingface.co/mosaicml/mpt-7b)
4. Falcon - Based off of the Falcon architecture with examples found [here](https://huggingface.co/tiiuae/falcon-7b)
5. Starcoder - Based off of the Starcoder architecture with examples found [here](https://huggingface.co/bigcode/starcoder)
6. Replit - Based off of the Replit architecture with examples found [here](https://huggingface.co/replit/replit-code-v1-3b)
7. Mistral - Based off of the Mistral architecture with examples found [here](https://huggingface.co/mistralai/Mistral-7B-v0.1)
8. Baichuan - Based off of the Baichuan architecture with examples found [here](https://huggingface.co/baichuan-inc/Baichuan-13B-Base)
9. BLOOM - Based off of the BLOOM architecture with examples found [here](https://huggingface.co/bigscience/bloom)
10. CodeShell - Based off of the CodeShell architecture with examples found [here](https://huggingface.co/CodeShell)
11. GPT-2 - Based off of the GPT-2 architecture with examples found [here](https://huggingface.co/models?sort=downloads&search=gpt2)
12. Orion - Based off of the Orion architecture with examples found [here](https://huggingface.co/Orion)
13. Persimmon - Based off of the Persimmon architecture with examples found [here](https://huggingface.co/Persimmon)
14. Phi - Based off of the Phi architecture with examples found [here](https://huggingface.co/Phi)
15. Plamo - Based off of the Plamo architecture with examples found [here](https://huggingface.co/Plamo)
16. Qwen - Based off of the Qwen architecture with examples found [here](https://huggingface.co/Qwen)
17. Refact - Based off of the Refact architecture with examples found [here](https://huggingface.co/Refact)
18. StableLM - Based off of the StableLM architecture with examples found [here](https://huggingface.co/stabilityai/stablelm-tuned-alpha-7b)

# Why so many different architectures? What differentiates them?

One of the major differences is license. Currently, the LLAMA based models are subject to a non-commercial license, whereas the GPTJ, MPT, Falcon, Starcoder, Replit, Mistral, Baichuan, BLOOM, CodeShell, GPT-2, Orion, Persimmon, Phi, Plamo, Qwen, Refact, and StableLM base models allow commercial usage. In the early advent of the recent explosion of activity in open source local models, the llama models have generally been seen as performing better, but that is changing quickly. Every week - even every day! - new models are released with some of the GPTJ, MPT, Falcon, Starcoder, Replit, Mistral, Baichuan, BLOOM, CodeShell, GPT-2, Orion, Persimmon, Phi, Plamo, Qwen, Refact, and StableLM models competitive in performance/quality with LLAMA. What's more, there are some very nice architectural innovations with the MPT, Falcon, Starcoder, Replit, Mistral, Baichuan, BLOOM, CodeShell, GPT-2, Orion, Persimmon, Phi, Plamo, Qwen, Refact, and StableLM models that could lead to new performance/quality gains.

# How does GPT4All make these models available for CPU inference?

By leveraging the ggml library written by Georgi Gerganov and a growing community of developers. There are currently multiple different versions of this library. The original github repo can be found [here](https://github.com/ggerganov/ggml), but the developer of the library has also created a LLAMA based version [here](https://github.com/ggerganov/llama.cpp). Currently, this backend is using the latter as a submodule.

# Does that mean GPT4All is compatible with all llama.cpp models and vice versa?

Unfortunately, no for three reasons:

1. The upstream [llama.cpp](https://github.com/ggerganov/llama.cpp) project has introduced [a compatibility breaking](https://github.com/ggerganov/llama.cpp/commit/b9fd7eee57df101d4a3e3eabc9fd6c2cb13c9ca1) re-quantization method recently. This is a breaking change that renders all previous models (including the ones that GPT4All uses) inoperative with newer versions of llama.cpp since that change.
2. The GPT4All backend has the llama.cpp submodule specifically pinned to a version prior to this breaking change.
3. The GPT4All backend currently supports MPT based models as an added feature. Neither llama.cpp nor the original ggml repo support this architecture as of this writing, however efforts are underway to make MPT available in the ggml repo which you can follow [here.](https://github.com/ggerganov/ggml/pull/145)

# What is being done to make them more compatible?

A few things. Number one, we are maintaining compatibility with our current model zoo by way of the submodule pinning. However, we are also exploring how we can update to newer versions of llama.cpp without breaking our current models. This might involve an additional magic header check or it could possibly involve keeping the currently pinned submodule and also adding a new submodule with later changes and differienting them with namespaces or some other manner. Investigations continue.

# What about GPU inference?

In newer versions of llama.cpp, there has been some added support for NVIDIA GPU's for inference. We're investigating how to incorporate this into our downloadable installers.

# Ok, so bottom line... how do I make my model on Hugging Face compatible with GPT4All ecosystem right now?

1. Check to make sure the Hugging Face model is available in one of our supported architectures
2. If it is, then you can use the conversion script inside of our pinned llama.cpp submodule for GPTJ and LLAMA based models
3. Or if your model is an MPT, Falcon, Starcoder, Replit, Mistral, Baichuan, BLOOM, CodeShell, GPT-2, Orion, Persimmon, Phi, Plamo, Qwen, Refact, or StableLM model you can use the conversion script located directly in this backend directory under the scripts subdirectory 

# Check back for updates as we'll try to keep this updated as things change!
