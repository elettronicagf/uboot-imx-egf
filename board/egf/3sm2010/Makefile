#
# Copyright 2019 NXP
#
# SPDX-License-Identifier:      GPL-2.0+
#

obj-y += 3sm2010.o
obj-y += gf_mux.o

ifdef CONFIG_SPL_BUILD
obj-y += spl.o
obj-$(CONFIG_FORCE_WID0727AB0101) += lpddr4_timing_wid0727ab0101.o
endif
