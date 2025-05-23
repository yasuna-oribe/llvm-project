; NOTE: Assertions have been autogenerated by utils/update_mir_test_checks.py
; RUN: llc -global-isel -mtriple=amdgcn-amd-amdhsa -stop-after=irtranslator -o - %s | FileCheck --check-prefix=FIXED %s

; Make sure arg1 is not allocated in v31, which is reserved for
; workitem IDs with -amdgpu-fixed-function-abi.

define void @void_a31i32_i32([31 x i32] %arg0, i32 %arg1) {
  ; FIXED-LABEL: name: void_a31i32_i32
  ; FIXED: bb.1 (%ir-block.0):
  ; FIXED-NEXT:   liveins: $vgpr0, $vgpr1, $vgpr2, $vgpr3, $vgpr4, $vgpr5, $vgpr6, $vgpr7, $vgpr8, $vgpr9, $vgpr10, $vgpr11, $vgpr12, $vgpr13, $vgpr14, $vgpr15, $vgpr16, $vgpr17, $vgpr18, $vgpr19, $vgpr20, $vgpr21, $vgpr22, $vgpr23, $vgpr24, $vgpr25, $vgpr26, $vgpr27, $vgpr28, $vgpr29, $vgpr30
  ; FIXED-NEXT: {{  $}}
  ; FIXED-NEXT:   [[COPY:%[0-9]+]]:_(s32) = COPY $vgpr0
  ; FIXED-NEXT:   [[COPY1:%[0-9]+]]:_(s32) = COPY $vgpr1
  ; FIXED-NEXT:   [[COPY2:%[0-9]+]]:_(s32) = COPY $vgpr2
  ; FIXED-NEXT:   [[COPY3:%[0-9]+]]:_(s32) = COPY $vgpr3
  ; FIXED-NEXT:   [[COPY4:%[0-9]+]]:_(s32) = COPY $vgpr4
  ; FIXED-NEXT:   [[COPY5:%[0-9]+]]:_(s32) = COPY $vgpr5
  ; FIXED-NEXT:   [[COPY6:%[0-9]+]]:_(s32) = COPY $vgpr6
  ; FIXED-NEXT:   [[COPY7:%[0-9]+]]:_(s32) = COPY $vgpr7
  ; FIXED-NEXT:   [[COPY8:%[0-9]+]]:_(s32) = COPY $vgpr8
  ; FIXED-NEXT:   [[COPY9:%[0-9]+]]:_(s32) = COPY $vgpr9
  ; FIXED-NEXT:   [[COPY10:%[0-9]+]]:_(s32) = COPY $vgpr10
  ; FIXED-NEXT:   [[COPY11:%[0-9]+]]:_(s32) = COPY $vgpr11
  ; FIXED-NEXT:   [[COPY12:%[0-9]+]]:_(s32) = COPY $vgpr12
  ; FIXED-NEXT:   [[COPY13:%[0-9]+]]:_(s32) = COPY $vgpr13
  ; FIXED-NEXT:   [[COPY14:%[0-9]+]]:_(s32) = COPY $vgpr14
  ; FIXED-NEXT:   [[COPY15:%[0-9]+]]:_(s32) = COPY $vgpr15
  ; FIXED-NEXT:   [[COPY16:%[0-9]+]]:_(s32) = COPY $vgpr16
  ; FIXED-NEXT:   [[COPY17:%[0-9]+]]:_(s32) = COPY $vgpr17
  ; FIXED-NEXT:   [[COPY18:%[0-9]+]]:_(s32) = COPY $vgpr18
  ; FIXED-NEXT:   [[COPY19:%[0-9]+]]:_(s32) = COPY $vgpr19
  ; FIXED-NEXT:   [[COPY20:%[0-9]+]]:_(s32) = COPY $vgpr20
  ; FIXED-NEXT:   [[COPY21:%[0-9]+]]:_(s32) = COPY $vgpr21
  ; FIXED-NEXT:   [[COPY22:%[0-9]+]]:_(s32) = COPY $vgpr22
  ; FIXED-NEXT:   [[COPY23:%[0-9]+]]:_(s32) = COPY $vgpr23
  ; FIXED-NEXT:   [[COPY24:%[0-9]+]]:_(s32) = COPY $vgpr24
  ; FIXED-NEXT:   [[COPY25:%[0-9]+]]:_(s32) = COPY $vgpr25
  ; FIXED-NEXT:   [[COPY26:%[0-9]+]]:_(s32) = COPY $vgpr26
  ; FIXED-NEXT:   [[COPY27:%[0-9]+]]:_(s32) = COPY $vgpr27
  ; FIXED-NEXT:   [[COPY28:%[0-9]+]]:_(s32) = COPY $vgpr28
  ; FIXED-NEXT:   [[COPY29:%[0-9]+]]:_(s32) = COPY $vgpr29
  ; FIXED-NEXT:   [[COPY30:%[0-9]+]]:_(s32) = COPY $vgpr30
  ; FIXED-NEXT:   [[FRAME_INDEX:%[0-9]+]]:_(p5) = G_FRAME_INDEX %fixed-stack.0
  ; FIXED-NEXT:   [[LOAD:%[0-9]+]]:_(s32) = G_LOAD [[FRAME_INDEX]](p5) :: (invariant load (s32) from %fixed-stack.0, align 16, addrspace 5)
  ; FIXED-NEXT:   [[DEF:%[0-9]+]]:_(p1) = G_IMPLICIT_DEF
  ; FIXED-NEXT:   G_STORE [[LOAD]](s32), [[DEF]](p1) :: (store (s32) into `ptr addrspace(1) poison`, addrspace 1)
  ; FIXED-NEXT:   SI_RETURN
  store i32 %arg1, ptr addrspace(1) poison
  ret void
}
