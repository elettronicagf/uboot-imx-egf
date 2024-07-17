#
# Copyright 2019 NXP
#
# SPDX-License-Identifier:      GPL-2.0+
#

obj-y += 3sm1008.o
obj-y += gf_mux.o

ifdef CONFIG_SPL_BUILD
obj-y += spl.o
obj-$(CONFIG_FORCE_WID0670AA0101) += ddr4_timing_wid0670aa0101.o
endif
