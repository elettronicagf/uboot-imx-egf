#
# Copyright 2019 NXP
# Copyright 2025 Elettronica GF s
#
# SPDX-License-Identifier:      GPL-2.0+
#

obj-y += 3sm2008.o
obj-y += gf_mux.o

ifdef CONFIG_SPL_BUILD
obj-y += spl.o
obj-$(CONFIG_FORCE_WID0700AA0101) += lpddr4_timing_wid0700aa0101.o
obj-$(CONFIG_FORCE_WID0700AB0101) += lpddr4_timing_wid0700ab0101.o
endif
