# TrulyNatural SDK version 7.3.0+709.g989ae21e43 build metadata

## Build host
```
Linux ubuntu-22-04 6.8.0-47-generic #47~22.04.1-Ubuntu SMP PREEMPT_DYNAMIC Wed Oct  2 16:16:55 UTC 2 x86_64 x86_64 x86_64 GNU/Linux
```

## Target platform

```
arm-arm-none-eabi
```

## Optional Features

| Module | OSS dependencies | Supported |
|:-------|:-----------------|:---------:|
| Speech-To-Text | ONNX Runtime | No |

**Note:** Optional modules are not available in applications
built with `-DSNSR_OMIT_OSS_COMPONENTS`.

## Open Source Components

This TrulyNatural SDK library includes Open Source Software:

| Library | License | URL |
|:--------|:--------|:----|
| TensorFlow Lite for Microcontrollers | Apache-2.0 | https://github.com/tensorflow/tflite-micro/blob/main/LICENSE |

See `oss-licenses/` in the TrulyNatural SDK installation directory for local copies of the license files.

You can avoid using any OSS components in an application linked against the TrulyNatural SDK by building with `-DSNSR_OMIT_OSS_COMPONENTS`.
This will disable all SDK features that depend on these.


## Compiler version

```
Product: Arm Development Studio Gold
Component: Arm Compiler for Embedded 6.21
Tool: armclang [5ec1f900]

Target: arm-arm-none-eabi
```

## Flags

```
 "/opt/arm/developmentstudio-2023.1/sw/ARMCompiler6.21/bin/armclang" \
   -cc1 -triple thumbv8.1m.main-arm-none-eabihf -E -disable-free \
   -clear-ast-before-backend -disable-llvm-verifier -discard-value-names \
   -main-file-name - -mrelocation-model static -mframe-pointer=none \
   -menable-no-infs -menable-no-nans -fmath-errno -fno-signed-zeros \
   -fdenormal-fp-math=preserve-sign,preserve-sign -ffp-contract=on \
   -fno-rounding-math -ffinite-math-only -mconstructor-aliases -target-cpu \
   cortex-m55 -target-feature -crc -target-feature -dotprod -target-feature \
   +dsp -target-feature +mve -target-feature +mve.fp -target-feature \
   +ras -target-feature -fp16fml -target-feature -bf16 -target-feature \
   -sb -target-feature -i8mm -target-feature +lob -target-feature -cdecp0 \
   -target-feature -cdecp1 -target-feature -cdecp2 -target-feature -cdecp3 \
   -target-feature -cdecp4 -target-feature -cdecp5 -target-feature -cdecp6 \
   -target-feature -cdecp7 -target-feature -pacbti -target-feature -fce \
   -target-feature -hwdiv-arm -target-feature +hwdiv -target-feature \
   +vfp2 -target-feature +vfp2sp -target-feature -vfp3 -target-feature \
   +vfp3d16 -target-feature +vfp3d16sp -target-feature -vfp3sp \
   -target-feature +fp16 -target-feature -vfp4 -target-feature +vfp4d16 \
   -target-feature +vfp4d16sp -target-feature -vfp4sp -target-feature \
   -fp-armv8 -target-feature +fp-armv8d16 -target-feature +fp-armv8d16sp \
   -target-feature -fp-armv8sp -target-feature +fullfp16 -target-feature \
   +fp64 -target-feature -d32 -target-feature -neon -target-feature \
   -crypto -target-feature -sha2 -target-feature -aes -target-abi \
   aapcs -mfloat-abi hard -debugger-tuning=gdb -fforce-dwarf-frame \
   -fdebug-compilation-dir=/home/sutton/Fluent/platform/none/arm-none-eabihf-sdk/4227-tflite-micro \
   -v -ffunction-sections -fdata-sections \
   -fcoverage-compilation-dir=/home/sutton/Fluent/platform/none/arm-none-eabihf-sdk/4227-tflite-micro \
   -resource-dir \
   /opt/arm/developmentstudio-2023.1/sw/ARMCompiler6.21/lib/clang/18 -D \
   "PLT_TUPLE=\"arm-arm-none-eabi\"" -D __FPU_PRESENT -D ARM_MATH_CM4 -D \
   LICENSE_NO_RTC -D NO_LICENSE_CHECK_IN_SNSR_RUN -D MATVEC_ARM_HELIUM \
   -D HEAP_ALIGN_SIZE_LOG2=4 -D KVSTORE_SRANDOM -D NO_RANDOM -D \
   ADD_MISSING_FUNCTIONS -D ADD_STRSEP -D ADD_STRDUP -D ENOENT=2 -D \
   ARMCM55 -D ARM_MATH_DSP -D ARM_MATH_LOOPUNROLL -D ARM_MATH_MVEI \
   -D ARM_NPU -D CPU_CORTEX_M55 -D "CPU_HEADER_FILE=\"ARMCM55.h\"" -D \
   TF_LITE_STATIC_MEMORY -D PRIdSZ=PRId32 -D PRIiSZ=PRIi32 -D PRIuSZ=PRIu32 \
   -D PRIxSZ=PRIx32 -D SCNdSZ=SCNd32 -D SCNiSZ=SCNi32 -D SCNuSZ=SCNu32 \
   -D SCNxSZ=SCNx32 -D USE_TFLITEMICRO -D TF_LITE_STATIC_MEMORY -D \
   NDEBUG -D OBFUSCATE=1 -D "SDK_API_PREFIX=\"snsr\"" -D LMQ_MAX_ORDER=20 -D \
   "PLT_BUILD=\"platform/none/arm-none-eabihf-sdk/4227-tflite-micro/Makefile.armDS2023.1-m55-infineon\"" \
   -D HAVE_CONFIG_H -D CONST84_H=CONST84 -D \
   "FLUENT_VERSION=\"8.0.0\"" -D USER_CACHE -D FLUENTSDK \
   -D OBFUSCATE_THIS -D LICENSE_THREAD_SAFE -D BUILD_YEAR=2024 -D \
   SDK_REQ_FLAGS=LICENSE_FLAG_32BIT|LICENSE_FLAG_ARM|LICENSE_FLAG_EMBEDDED|LICENSE_FLAG_LINUX \
   -internal-isystem \
   /opt/arm/developmentstudio-2023.1/sw/ARMCompiler6.21/bin/../include \
   -nobuiltininc -Os -Wno-format -Wno-incompatible-pointer-types \
   -ferror-limit 19 -fvisibility=hidden -flax-vector-conversions=integer \
   -fshort-enums -fno-signed-char -fwchar-type=short -fno-signed-wchar \
   -fgnuc-version=4.2.1 -fsuppress-licensing -vectorize-loops -vectorize-slp \
   -aggressive-jump-threading -o - -x c - \
  Arm Compiler for Embedded 6.21 -cc1 default target aarch64-arm-none-eabi
```

## Macros

```
#define ADD_MISSING_FUNCTIONS 1
#define ADD_STRDUP 1
#define ADD_STRSEP 1
#define __APCS_32__ 1
#define __arm 1
#define __arm__ 1
#define __ARM_32BIT_STATE 1
#define __ARM_ACLE 200
#define __ARM_ARCH 8
#define __ARM_ARCH_8_1M_MAIN__ 1
#define __ARM_ARCH_EXT_IDIV__ 1
#define __ARM_ARCH_ISA_THUMB 2
#define __ARM_ARCH_PROFILE 'M'
#define __ARMCC_VERSION 6210000
#define ARMCM55 1
#define __ARMCOMPILER_VERSION 6210000
#define __ARM_EABI__ 1
#define __ARMEL__ 1
#define __ARM_FEATURE_CLZ 1
#define __ARM_FEATURE_CMSE 1
#define __ARM_FEATURE_DIRECTED_ROUNDING 1
#define __ARM_FEATURE_DSP 1
#define __ARM_FEATURE_FMA 1
#define __ARM_FEATURE_FP16_SCALAR_ARITHMETIC 1
#define __ARM_FEATURE_IDIV 1
#define __ARM_FEATURE_LDREX 0xf
#define __ARM_FEATURE_MVE 3
#define __ARM_FEATURE_NUMERIC_MAXMIN 1
#define __ARM_FEATURE_QBIT 1
#define __ARM_FEATURE_SAT 1
#define __ARM_FEATURE_SIMD32 1
#define __ARM_FEATURE_UNALIGNED 1
#define __ARM_FP 0xe
#define __ARM_FP16_ARGS 1
#define __ARM_FP16_FORMAT_IEEE 1
#define __ARM_FPV5__ 1
#define ARM_MATH_CM4 1
#define ARM_MATH_DSP 1
#define ARM_MATH_LOOPUNROLL 1
#define ARM_MATH_MVEI 1
#define __ARM_NO_IMAGINARY_TYPE 1
#define ARM_NPU 1
#define __ARM_PCS 1
#define __ARM_PCS_VFP 1
#define __ARM_PROMISE __builtin_assume
#define __ARM_SIZEOF_MINIMAL_ENUM 1
#define __ARM_SIZEOF_WCHAR_T 2
#define __ARM_VFPV2__ 1
#define __ARM_VFPV3__ 1
#define __ARM_VFPV4__ 1
#define __ATOMIC_ACQ_REL 4
#define __ATOMIC_ACQUIRE 2
#define __ATOMIC_CONSUME 1
#define __ATOMIC_RELAXED 0
#define __ATOMIC_RELEASE 3
#define __ATOMIC_SEQ_CST 5
#define __BIGGEST_ALIGNMENT__ 8
#define __BITINT_MAXWIDTH__ 128
#define __BOOL_WIDTH__ 8
#define BUILD_YEAR 2024
#define __BYTE_ORDER__ __ORDER_LITTLE_ENDIAN__
#define __CHAR16_TYPE__ unsigned short
#define __CHAR32_TYPE__ unsigned int
#define __CHAR_BIT__ 8
#define __CHAR_UNSIGNED__ 1
#define __clang__ 1
#define __CLANG_ATOMIC_BOOL_LOCK_FREE 2
#define __CLANG_ATOMIC_CHAR16_T_LOCK_FREE 2
#define __CLANG_ATOMIC_CHAR32_T_LOCK_FREE 2
#define __CLANG_ATOMIC_CHAR_LOCK_FREE 2
#define __CLANG_ATOMIC_INT_LOCK_FREE 2
#define __CLANG_ATOMIC_LLONG_LOCK_FREE 1
#define __CLANG_ATOMIC_LONG_LOCK_FREE 2
#define __CLANG_ATOMIC_POINTER_LOCK_FREE 2
#define __CLANG_ATOMIC_SHORT_LOCK_FREE 2
#define __CLANG_ATOMIC_WCHAR_T_LOCK_FREE 2
#define __clang_literal_encoding__ "UTF-8"
#define __clang_major__ 18
#define __clang_minor__ 0
#define __clang_patchlevel__ 0
#define __clang_version__ "18.0.0 "
#define __clang_wide_literal_encoding__ "UTF-16"
#define CONST84_H CONST84
#define __CONSTANT_CFSTRINGS__ 1
#define CPU_CORTEX_M55 1
#define CPU_HEADER_FILE "ARMCM55.h"
#define __DBL_DECIMAL_DIG__ 17
#define __DBL_DENORM_MIN__ 4.9406564584124654e-324
#define __DBL_DIG__ 15
#define __DBL_EPSILON__ 2.2204460492503131e-16
#define __DBL_HAS_DENORM__ 1
#define __DBL_HAS_INFINITY__ 1
#define __DBL_HAS_QUIET_NAN__ 1
#define __DBL_MANT_DIG__ 53
#define __DBL_MAX_10_EXP__ 308
#define __DBL_MAX__ 1.7976931348623157e+308
#define __DBL_MAX_EXP__ 1024
#define __DBL_MIN_10_EXP__ (-307)
#define __DBL_MIN__ 2.2250738585072014e-308
#define __DBL_MIN_EXP__ (-1021)
#define __DECIMAL_DIG__ __LDBL_DECIMAL_DIG__
#define __ELF__ 1
#define ENOENT 2
#define __ESCAPE__ 
#define __FINITE_MATH_ONLY__ 1
#define __FLT16_DECIMAL_DIG__ 5
#define __FLT16_DENORM_MIN__ 5.9604644775390625e-8F16
#define __FLT16_DIG__ 3
#define __FLT16_EPSILON__ 9.765625e-4F16
#define __FLT16_HAS_DENORM__ 1
#define __FLT16_HAS_INFINITY__ 1
#define __FLT16_HAS_QUIET_NAN__ 1
#define __FLT16_MANT_DIG__ 11
#define __FLT16_MAX_10_EXP__ 4
#define __FLT16_MAX__ 6.5504e+4F16
#define __FLT16_MAX_EXP__ 16
#define __FLT16_MIN_10_EXP__ (-4)
#define __FLT16_MIN__ 6.103515625e-5F16
#define __FLT16_MIN_EXP__ (-13)
#define __FLT_DECIMAL_DIG__ 9
#define __FLT_DENORM_MIN__ 1.40129846e-45F
#define __FLT_DIG__ 6
#define __FLT_EPSILON__ 1.19209290e-7F
#define __FLT_HAS_DENORM__ 1
#define __FLT_HAS_INFINITY__ 1
#define __FLT_HAS_QUIET_NAN__ 1
#define __FLT_MANT_DIG__ 24
#define __FLT_MAX_10_EXP__ 38
#define __FLT_MAX__ 3.40282347e+38F
#define __FLT_MAX_EXP__ 128
#define __FLT_MIN_10_EXP__ (-37)
#define __FLT_MIN__ 1.17549435e-38F
#define __FLT_MIN_EXP__ (-125)
#define __FLT_RADIX__ 2
#define FLUENTSDK 1
#define FLUENT_VERSION "8.0.0"
#define __FPCLASS_NEGINF 0x0004
#define __FPCLASS_NEGNORMAL 0x0008
#define __FPCLASS_NEGSUBNORMAL 0x0010
#define __FPCLASS_NEGZERO 0x0020
#define __FPCLASS_POSINF 0x0200
#define __FPCLASS_POSNORMAL 0x0100
#define __FPCLASS_POSSUBNORMAL 0x0080
#define __FPCLASS_POSZERO 0x0040
#define __FPCLASS_QNAN 0x0002
#define __FPCLASS_SNAN 0x0001
#define __FPU_PRESENT 1
#define __GCC_ATOMIC_BOOL_LOCK_FREE 2
#define __GCC_ATOMIC_CHAR16_T_LOCK_FREE 2
#define __GCC_ATOMIC_CHAR32_T_LOCK_FREE 2
#define __GCC_ATOMIC_CHAR_LOCK_FREE 2
#define __GCC_ATOMIC_INT_LOCK_FREE 2
#define __GCC_ATOMIC_LLONG_LOCK_FREE 1
#define __GCC_ATOMIC_LONG_LOCK_FREE 2
#define __GCC_ATOMIC_POINTER_LOCK_FREE 2
#define __GCC_ATOMIC_SHORT_LOCK_FREE 2
#define __GCC_ATOMIC_TEST_AND_SET_TRUEVAL 1
#define __GCC_ATOMIC_WCHAR_T_LOCK_FREE 2
#define __GCC_HAVE_SYNC_COMPARE_AND_SWAP_1 1
#define __GCC_HAVE_SYNC_COMPARE_AND_SWAP_2 1
#define __GCC_HAVE_SYNC_COMPARE_AND_SWAP_4 1
#define __GCC_HAVE_SYNC_COMPARE_AND_SWAP_8 1
#define __GNUC__ 4
#define __GNUC_MINOR__ 2
#define __GNUC_PATCHLEVEL__ 1
#define __GNUC_STDC_INLINE__ 1
#define __GXX_ABI_VERSION 1002
#define HAVE_CONFIG_H 1
#define HEAP_ALIGN_SIZE_LOG2 4
#define __I__ 1.0if
#define __ILP32__ 1
#define _ILP32 1
#define __INT16_C_SUFFIX__ 
#define __INT16_FMTd__ "hd"
#define __INT16_FMTi__ "hi"
#define __INT16_MAX__ 32767
#define __INT16_TYPE__ short
#define __INT32_C_SUFFIX__ 
#define __INT32_FMTd__ "d"
#define __INT32_FMTi__ "i"
#define __INT32_MAX__ 2147483647
#define __INT32_TYPE__ int
#define __INT64_C_SUFFIX__ LL
#define __INT64_FMTd__ "lld"
#define __INT64_FMTi__ "lli"
#define __INT64_MAX__ 9223372036854775807LL
#define __INT64_TYPE__ long long int
#define __INT8_C_SUFFIX__ 
#define __INT8_FMTd__ "hhd"
#define __INT8_FMTi__ "hhi"
#define __INT8_MAX__ 127
#define __INT8_TYPE__ signed char
#define __INT_FAST16_FMTd__ "hd"
#define __INT_FAST16_FMTi__ "hi"
#define __INT_FAST16_MAX__ 32767
#define __INT_FAST16_TYPE__ short
#define __INT_FAST16_WIDTH__ 16
#define __INT_FAST32_FMTd__ "d"
#define __INT_FAST32_FMTi__ "i"
#define __INT_FAST32_MAX__ 2147483647
#define __INT_FAST32_TYPE__ int
#define __INT_FAST32_WIDTH__ 32
#define __INT_FAST64_FMTd__ "lld"
#define __INT_FAST64_FMTi__ "lli"
#define __INT_FAST64_MAX__ 9223372036854775807LL
#define __INT_FAST64_TYPE__ long long int
#define __INT_FAST64_WIDTH__ 64
#define __INT_FAST8_FMTd__ "hhd"
#define __INT_FAST8_FMTi__ "hhi"
#define __INT_FAST8_MAX__ 127
#define __INT_FAST8_TYPE__ signed char
#define __INT_FAST8_WIDTH__ 8
#define __INT_LEAST16_FMTd__ "hd"
#define __INT_LEAST16_FMTi__ "hi"
#define __INT_LEAST16_MAX__ 32767
#define __INT_LEAST16_TYPE__ short
#define __INT_LEAST16_WIDTH__ 16
#define __INT_LEAST32_FMTd__ "d"
#define __INT_LEAST32_FMTi__ "i"
#define __INT_LEAST32_MAX__ 2147483647
#define __INT_LEAST32_TYPE__ int
#define __INT_LEAST32_WIDTH__ 32
#define __INT_LEAST64_FMTd__ "lld"
#define __INT_LEAST64_FMTi__ "lli"
#define __INT_LEAST64_MAX__ 9223372036854775807LL
#define __INT_LEAST64_TYPE__ long long int
#define __INT_LEAST64_WIDTH__ 64
#define __INT_LEAST8_FMTd__ "hhd"
#define __INT_LEAST8_FMTi__ "hhi"
#define __INT_LEAST8_MAX__ 127
#define __INT_LEAST8_TYPE__ signed char
#define __INT_LEAST8_WIDTH__ 8
#define __INT_MAX__ 2147483647
#define __INTMAX_C_SUFFIX__ LL
#define __INTMAX_FMTd__ "lld"
#define __INTMAX_FMTi__ "lli"
#define __INTMAX_MAX__ 9223372036854775807LL
#define __INTMAX_TYPE__ long long int
#define __INTMAX_WIDTH__ 64
#define __INTPTR_FMTd__ "d"
#define __INTPTR_FMTi__ "i"
#define __INTPTR_MAX__ 2147483647
#define __INTPTR_TYPE__ int
#define __INTPTR_WIDTH__ 32
#define __INT_WIDTH__ 32
#define KVSTORE_SRANDOM 1
#define __LDBL_DECIMAL_DIG__ 17
#define __LDBL_DENORM_MIN__ 4.9406564584124654e-324L
#define __LDBL_DIG__ 15
#define __LDBL_EPSILON__ 2.2204460492503131e-16L
#define __LDBL_HAS_DENORM__ 1
#define __LDBL_HAS_INFINITY__ 1
#define __LDBL_HAS_QUIET_NAN__ 1
#define __LDBL_MANT_DIG__ 53
#define __LDBL_MAX_10_EXP__ 308
#define __LDBL_MAX__ 1.7976931348623157e+308L
#define __LDBL_MAX_EXP__ 1024
#define __LDBL_MIN_10_EXP__ (-307)
#define __LDBL_MIN__ 2.2250738585072014e-308L
#define __LDBL_MIN_EXP__ (-1021)
#define LICENSE_NO_RTC 1
#define LICENSE_THREAD_SAFE 1
#define __LITTLE_ENDIAN__ 1
#define __LLONG_WIDTH__ 64
#define __llvm__ 1
#define LMQ_MAX_ORDER 20
#define __LONG_LONG_MAX__ 9223372036854775807LL
#define __LONG_MAX__ 2147483647L
#define __LONG_WIDTH__ 32
#define MATVEC_ARM_HELIUM 1
#define NDEBUG 1
#define NO_LICENSE_CHECK_IN_SNSR_RUN 1
#define NO_RANDOM 1
#define OBFUSCATE 1
#define OBFUSCATE_THIS 1
#define __OBJC_BOOL_IS_BOOL 0
#define __OPENCL_MEMORY_SCOPE_ALL_SVM_DEVICES 3
#define __OPENCL_MEMORY_SCOPE_DEVICE 2
#define __OPENCL_MEMORY_SCOPE_SUB_GROUP 4
#define __OPENCL_MEMORY_SCOPE_WORK_GROUP 1
#define __OPENCL_MEMORY_SCOPE_WORK_ITEM 0
#define __OPTIMIZE__ 1
#define __OPTIMIZE_SIZE__ 1
#define __ORDER_BIG_ENDIAN__ 4321
#define __ORDER_LITTLE_ENDIAN__ 1234
#define __ORDER_PDP_ENDIAN__ 3412
#define PLT_BUILD "platform/none/arm-none-eabihf-sdk/4227-tflite-micro/Makefile.armDS2023.1-m55-infineon"
#define PLT_TUPLE "arm-arm-none-eabi"
#define __POINTER_WIDTH__ 32
#define __PRAGMA_REDEFINE_EXTNAME 1
#define PRIdSZ PRId32
#define PRIiSZ PRIi32
#define PRIuSZ PRIu32
#define PRIxSZ PRIx32
#define __PTRDIFF_FMTd__ "d"
#define __PTRDIFF_FMTi__ "i"
#define __PTRDIFF_MAX__ 2147483647
#define __PTRDIFF_TYPE__ int
#define __PTRDIFF_WIDTH__ 32
#define __REGISTER_PREFIX__ 
#define __SCHAR_MAX__ 127
#define SCNdSZ SCNd32
#define SCNiSZ SCNi32
#define SCNuSZ SCNu32
#define SCNxSZ SCNx32
#define SDK_API_PREFIX "snsr"
#define SDK_REQ_FLAGS LICENSE_FLAG_32BIT|LICENSE_FLAG_ARM|LICENSE_FLAG_EMBEDDED|LICENSE_FLAG_LINUX
#define __SHRT_MAX__ 32767
#define __SHRT_WIDTH__ 16
#define __SIG_ATOMIC_MAX__ 2147483647
#define __SIG_ATOMIC_WIDTH__ 32
#define __SIZE_FMTo__ "o"
#define __SIZE_FMTu__ "u"
#define __SIZE_FMTx__ "x"
#define __SIZE_FMTX__ "X"
#define __SIZE_MAX__ 4294967295U
#define __SIZEOF_DOUBLE__ 8
#define __SIZEOF_FLOAT__ 4
#define __SIZEOF_INT__ 4
#define __SIZEOF_LONG__ 4
#define __SIZEOF_LONG_DOUBLE__ 8
#define __SIZEOF_LONG_LONG__ 8
#define __SIZEOF_POINTER__ 4
#define __SIZEOF_PTRDIFF_T__ 4
#define __SIZEOF_SHORT__ 2
#define __SIZEOF_SIZE_T__ 4
#define __SIZEOF_WCHAR_T__ 2
#define __SIZEOF_WINT_T__ 4
#define __SIZE_TYPE__ unsigned int
#define __SIZE_WIDTH__ 32
#define __STDC__ 1
#define __STDC_HOSTED__ 1
#define __STDC_UTF_16__ 1
#define __STDC_UTF_32__ 1
#define __STDC_VERSION__ 201710L
#define TF_LITE_STATIC_MEMORY 1
#define __thumb__ 1
#define __thumb2__ 1
#define __THUMBEL__ 1
#define __THUMB_INTERWORK__ 1
#define __UINT16_C_SUFFIX__ 
#define __UINT16_FMTo__ "ho"
#define __UINT16_FMTu__ "hu"
#define __UINT16_FMTx__ "hx"
#define __UINT16_FMTX__ "hX"
#define __UINT16_MAX__ 65535
#define __UINT16_TYPE__ unsigned short
#define __UINT32_C_SUFFIX__ U
#define __UINT32_FMTo__ "o"
#define __UINT32_FMTu__ "u"
#define __UINT32_FMTx__ "x"
#define __UINT32_FMTX__ "X"
#define __UINT32_MAX__ 4294967295U
#define __UINT32_TYPE__ unsigned int
#define __UINT64_C_SUFFIX__ ULL
#define __UINT64_FMTo__ "llo"
#define __UINT64_FMTu__ "llu"
#define __UINT64_FMTx__ "llx"
#define __UINT64_FMTX__ "llX"
#define __UINT64_MAX__ 18446744073709551615ULL
#define __UINT64_TYPE__ long long unsigned int
#define __UINT8_C_SUFFIX__ 
#define __UINT8_FMTo__ "hho"
#define __UINT8_FMTu__ "hhu"
#define __UINT8_FMTx__ "hhx"
#define __UINT8_FMTX__ "hhX"
#define __UINT8_MAX__ 255
#define __UINT8_TYPE__ unsigned char
#define __UINT_FAST16_FMTo__ "ho"
#define __UINT_FAST16_FMTu__ "hu"
#define __UINT_FAST16_FMTx__ "hx"
#define __UINT_FAST16_FMTX__ "hX"
#define __UINT_FAST16_MAX__ 65535
#define __UINT_FAST16_TYPE__ unsigned short
#define __UINT_FAST32_FMTo__ "o"
#define __UINT_FAST32_FMTu__ "u"
#define __UINT_FAST32_FMTx__ "x"
#define __UINT_FAST32_FMTX__ "X"
#define __UINT_FAST32_MAX__ 4294967295U
#define __UINT_FAST32_TYPE__ unsigned int
#define __UINT_FAST64_FMTo__ "llo"
#define __UINT_FAST64_FMTu__ "llu"
#define __UINT_FAST64_FMTx__ "llx"
#define __UINT_FAST64_FMTX__ "llX"
#define __UINT_FAST64_MAX__ 18446744073709551615ULL
#define __UINT_FAST64_TYPE__ long long unsigned int
#define __UINT_FAST8_FMTo__ "hho"
#define __UINT_FAST8_FMTu__ "hhu"
#define __UINT_FAST8_FMTx__ "hhx"
#define __UINT_FAST8_FMTX__ "hhX"
#define __UINT_FAST8_MAX__ 255
#define __UINT_FAST8_TYPE__ unsigned char
#define __UINT_LEAST16_FMTo__ "ho"
#define __UINT_LEAST16_FMTu__ "hu"
#define __UINT_LEAST16_FMTx__ "hx"
#define __UINT_LEAST16_FMTX__ "hX"
#define __UINT_LEAST16_MAX__ 65535
#define __UINT_LEAST16_TYPE__ unsigned short
#define __UINT_LEAST32_FMTo__ "o"
#define __UINT_LEAST32_FMTu__ "u"
#define __UINT_LEAST32_FMTx__ "x"
#define __UINT_LEAST32_FMTX__ "X"
#define __UINT_LEAST32_MAX__ 4294967295U
#define __UINT_LEAST32_TYPE__ unsigned int
#define __UINT_LEAST64_FMTo__ "llo"
#define __UINT_LEAST64_FMTu__ "llu"
#define __UINT_LEAST64_FMTx__ "llx"
#define __UINT_LEAST64_FMTX__ "llX"
#define __UINT_LEAST64_MAX__ 18446744073709551615ULL
#define __UINT_LEAST64_TYPE__ long long unsigned int
#define __UINT_LEAST8_FMTo__ "hho"
#define __UINT_LEAST8_FMTu__ "hhu"
#define __UINT_LEAST8_FMTx__ "hhx"
#define __UINT_LEAST8_FMTX__ "hhX"
#define __UINT_LEAST8_MAX__ 255
#define __UINT_LEAST8_TYPE__ unsigned char
#define __UINTMAX_C_SUFFIX__ ULL
#define __UINTMAX_FMTo__ "llo"
#define __UINTMAX_FMTu__ "llu"
#define __UINTMAX_FMTx__ "llx"
#define __UINTMAX_FMTX__ "llX"
#define __UINTMAX_MAX__ 18446744073709551615ULL
#define __UINTMAX_TYPE__ long long unsigned int
#define __UINTMAX_WIDTH__ 64
#define __UINTPTR_FMTo__ "o"
#define __UINTPTR_FMTu__ "u"
#define __UINTPTR_FMTx__ "x"
#define __UINTPTR_FMTX__ "X"
#define __UINTPTR_MAX__ 4294967295U
#define __UINTPTR_TYPE__ unsigned int
#define __UINTPTR_WIDTH__ 32
#define USER_CACHE 1
#define __USER_LABEL_PREFIX__ 
#define _USE_STATIC_INLINE 1
#define USE_TFLITEMICRO 1
#define __VERSION__ "Clang 18.0.0"
#define __VFP_FP__ 1
#define __WCHAR_MAX__ 65535
#define __WCHAR_TYPE__ unsigned short
#define __WCHAR_UNSIGNED__ 1
#define __WCHAR_WIDTH__ 16
#define __WINT_MAX__ 2147483647
#define __WINT_TYPE__ int
#define __WINT_WIDTH__ 32
```
