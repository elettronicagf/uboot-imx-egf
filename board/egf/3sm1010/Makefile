#
# Copyright 2019 NXP
#
# SPDX-License-Identifier:      GPL-2.0+
#

obj-y += 3sm1010.o
obj-y += gf_mux.o

ifdef CONFIG_SPL_BUILD
obj-y += spl.o
obj-$(CONFIG_FORCE_WID0820AA0101) += lpddr4x_timing_wid0820aa0101.o
endif
