/*
 * pci.c
 *
 * Purpose:		PCI configuration for the Coldfire builtin PCI bridge.
 *
 * Notes:
 *
 * This file is part of BaS_gcc.
 *
 * BaS_gcc is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * BaS_gcc is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with BaS_gcc.  If not, see <http://www.gnu.org/licenses/>.
 *
 *  Created on: 08.01.2013
 *      Author: Markus Froeschle
 */

#include <MCF5475.h>
#include "pci.h"
#include "stdint.h"
#include "bas_printf.h"
#include "util.h"
#include "wait.h"

static struct pci_class
{
	int classcode;
	char *description;
} pci_classes[] =
{
	{ 0x00, "device was built prior definition of the class code field" },
	{ 0x01, "Mass Storage Controller" },
	{ 0x02, "Network Controller" },
	{ 0x03, "Display Controller" },
	{ 0x04, "Multimedia Controller" },
	{ 0x05, "Memory Controller" },
	{ 0x06, "Bridge Device" },
	{ 0x07, "Simple Communication Controller" },
	{ 0x08, "Base System Peripherial" },
	{ 0x09, "Input Device" },
	{ 0x0a, "Docking Station" },
	{ 0x0b, "Processor" },
	{ 0x0c, "Serial Bus Controller" },
	{ 0x0d, "Wireless Controller" },
	{ 0x0e, "Intelligent I/O Controller" },
	{ 0x0f, "Satellite Communication Controller" },
	{ 0x10, "Encryption/Decryption Controller" },
	{ 0x11, "Data Acquisition and Signal Processing Controller" },
	{ 0xff, "Device does not fit any defined class" },
};
static int num_classes = sizeof(pci_classes) / sizeof(struct pci_class);

static struct resource_descriptor resource_descriptors[16];

static char *device_class(int classcode)
{
	int i;

	for (i = 0; i < num_classes; i++)
	{
		if (pci_classes[i].classcode == classcode)
		{
			return pci_classes[i].description;
		}
	}
	return "not found";
}

uint32_t pci_read_config_longword(uint16_t handle, uint16_t offset)
{
	uint32_t value;
	uint16_t bus = PCI_BUS_FROM_HANDLE(handle);
	uint16_t slot = PCI_SLOT_FROM_HANDLE(handle);
	uint16_t function = PCI_FUNCTION_FROM_HANDLE(handle);

	/* clear PCI status/command register */
	MCF_PCI_PCISCR = MCF_PCI_PCISCR_PE |		/* clear parity error bit */
				MCF_PCI_PCISCR_SE |					/* clear system error */
				MCF_PCI_PCISCR_MA |					/* clear master abort */
				MCF_PCI_PCISCR_TR |					/* clear target abort */
				MCF_PCI_PCISCR_TS |					/* clear target abort signalling (as target) */
				MCF_PCI_PCISCR_DP;					/* clear parity error */
	
	//(void) MCF_PCI_PCISCR;
	wait(1000);

	//xprintf("PCISCR before config cycle: %lx\r\n", MCF_PCI_PCISCR);

	/* initiate PCI configuration access to device */

	MCF_PCI_PCICAR = MCF_PCI_PCICAR_E |			/* enable configuration access special cycle */
			MCF_PCI_PCICAR_BUSNUM(bus) |
			MCF_PCI_PCICAR_DEVNUM(slot) |			/* device number, devices 0 - 9 are reserved */
			MCF_PCI_PCICAR_FUNCNUM(function) |	/* function number */
			MCF_PCI_PCICAR_DWORD(offset / 4);

	wait(1000);
	value =  * (volatile uint32_t *) PCI_IO_OFFSET;	/* access device */
	//xprintf("pci_read_config_longword(%d (bus=%d, slot=%d, function=%d), %d) = %d\r\n", handle, bus, slot, function, offset, swpl(value));

	return swpl(value);
}

uint16_t pci_read_config_word(uint16_t handle, uint16_t offset)
{
   uint32_t value;

   value = pci_read_config_longword(handle, offset / 2);
   return((value >> (1 - offset % 2) * 8) & 0xffff);
}

uint8_t pci_read_config_byte(uint16_t handle, uint16_t offset)
{
	uint32_t value;
	
	value = pci_read_config_longword(handle, offset / 4);
	return ((value >> (3 - offset % 4) * 8) & 0xff);
}

void pci_write_config_longword(uint16_t handle, uint16_t offset, uint32_t value)
{
	uint16_t bus = PCI_BUS_FROM_HANDLE(handle);
	uint16_t slot = PCI_SLOT_FROM_HANDLE(handle);
	uint16_t function = PCI_FUNCTION_FROM_HANDLE(handle);

	/* clear PCI status/command register */
	MCF_PCI_PCISCR = MCF_PCI_PCISCR_PE |		/* clear parity error bit */
				MCF_PCI_PCISCR_SE |					/* clear system error */
				MCF_PCI_PCISCR_MA |					/* clear master abort */
				MCF_PCI_PCISCR_TR |					/* clear target abort */
				MCF_PCI_PCISCR_TS |					/* clear target abort signalling (as target) */
				MCF_PCI_PCISCR_DP;					/* clear parity error */

	//(void) MCF_PCI_PCISCR;
	wait(1000);

	//xprintf("PCISCR before config cycle: %lx\r\n", MCF_PCI_PCISCR);

	/* initiate PCI configuration access to device */

	MCF_PCI_PCICAR = MCF_PCI_PCICAR_E |				/* enable configuration access special cycle */
			MCF_PCI_PCICAR_BUSNUM(bus) |
			MCF_PCI_PCICAR_DEVNUM(slot) |			/* device number, devices 0 - 9 are reserved */
			MCF_PCI_PCICAR_FUNCNUM(function) |		/* function number */
			MCF_PCI_PCICAR_DWORD(offset / 4);

	wait(1000);
	* (volatile uint32_t *) PCI_IO_OFFSET = swpl(value);	/* access device */
}

struct resource_descriptor *pci_get_resource(uint16_t handle)
{
	/* TODO: implement */

	return (struct resource_descriptor *) 0L;
}

int pci_find_device(uint16_t device_id, uint16_t vendor_id, int index)
{
	uint16_t bus;
	uint16_t slot;
	uint16_t function;
	uint16_t pos = 0;
	int handle;

	for (bus = 0; bus < 1; bus++)
	{
		for (slot = 0; slot < 32; slot++)
		{
			for (function = 0; function < 8; function++)
			{
				uint32_t value;

				handle = PCI_HANDLE(bus, slot, function);
				value = pci_read_config_longword(handle, 0);
				if (value != 0xffffffff)	/* we have a device at this position */
				{
					if (vendor_id == 0xffff)	/* ignore vendor id */
					{
						return handle;
					}
					else if (PCI_VENDOR_ID(value) == vendor_id && PCI_DEVICE_ID(value) == device_id)
					{
						if (pos == index)
							return handle;
						pos++;
					}
				}
			}
		}
	}
	return PCI_DEVICE_NOT_FOUND;
}


static uint32_t mem_address = PCI_MEMORY_OFFSET;
static uint32_t io_address = PCI_IO_OFFSET;

void pci_device_config(uint16_t bus, uint16_t slot, uint16_t function)
{
	uint32_t address;
	uint16_t handle;
	int i;

	for (i = 0; i < 6; i++)		/* for all bars */
	{
		uint32_t value;

		handle = bus << 8 | slot << 5 | function;
		value = pci_read_config_longword(handle, 0x10 + i);			/* read BAR value */
		pci_write_config_longword(handle, 0x10 + i, 0xffffffff);	/* write all bits */
		address = pci_read_config_longword(handle, 0x10 + i);		/* read back value */

		if (address)	/* is bar in use? */
		{
			//xprintf("%s region found with base address %08x, size = %x\r\n",
				//(IS_PCI_MEM_BAR(value) ? "Memory" : "I/O"),
				//(IS_PCI_MEM_BAR(value) ? PCI_MEMBAR_ADR(value) : PCI_IOBAR_ADR(value)),
				//(IS_PCI_MEM_BAR(value) ? ~(address & 0xfffffff0) + 1 : ~(address & 0xfffffffc) + 1));

			/* adjust base address to alignment requirements */
			if (IS_PCI_MEM_BAR(value))
			{
				int size = ~(address & 0xfffffff0) + 1;

				mem_address = (mem_address + size - 1) & ~(size - 1);
				pci_write_config_longword(handle, 0x10 + i, mem_address);
				value = pci_read_config_longword(handle, 0x10 + i);
				//xprintf("BAR[%d] configured to %08x, size %x\r\n", i, value, size);
				mem_address += size;
			}
			else if (IS_PCI_IO_BAR(value))
			{
				int size = ~(address & 0xfffffffc) + 1;

				io_address = (io_address + size - 1) & ~(size - 1);
				pci_write_config_longword(handle, 0x10 + i, io_address);
				value = pci_read_config_longword(handle, 0x10 + i);
				//xprintf("BAR[%d] mapped to %08x, size %x\r\n", i, value, size);
				io_address += size;
			}
		}
	}
}

void pci_scan(void)
{
	uint16_t bus;
	uint16_t slot;
	uint16_t function;
	uint16_t i;

	xprintf("\r\nPCI bus scan...\r\n\r\n");
	xprintf(" Bus|Slot|Func|Vndr|Dev |\r\n");
	xprintf("----+----+----|----+----|\r\n");
	for (bus = 0; bus < 1; bus++)
	{
		for (slot = 0; slot < 32; slot++)
		{
			for (function = 0; function < 8; function++)
			{
				uint32_t value;
				uint16_t handle = 0 | bus << 8 | slot << 5 | function;

				value = pci_read_config_longword(handle, 0);
				if (value != 0xffffffff)
				{
					xprintf(" %02x | %02x | %02x |%04x|%04x| %s\r\n", bus, slot, function,
							PCI_VENDOR_ID(value),
							PCI_DEVICE_ID(value),
							device_class(pci_read_config_longword(handle, 0x08) >> 24 & 0xff));

					if (PCI_VENDOR_ID(value) != 0x1057 && PCI_DEVICE_ID(value) != 0x5806) /* do not configure bridge */
					{
						pci_device_config(bus, slot, function);
					}

					for (i = 0; i < 0x40; i += 4)
					{
						value = pci_read_config_longword(handle, i);
						//xprintf("register %02x value= %08x\r\n", i, value);
					}
					/* test for multi-function device to avoid ghost device detects */
					value = pci_read_config_longword(handle, 0x0c);	
					if (function == 0 && !(PCI_HEADER_TYPE(value) & 0x80))	/* no multi function device */
						function = 8;
				}
			}
		}
	}
	xprintf("\r\n...finished\r\n");
}

/* start of PCI initialization code */
void init_eport(void)
{
	/* concigure IRQ1-7 pins on EPORT falling edge triggered */
	MCF_EPORT_EPPAR = MCF_EPORT_EPPAR_EPPA7(MCF_EPORT_EPPAR_FALLING) +
			MCF_EPORT_EPPAR_EPPA6(MCF_EPORT_EPPAR_FALLING) +
			MCF_EPORT_EPPAR_EPPA5(MCF_EPORT_EPPAR_FALLING) +
			MCF_EPORT_EPPAR_EPPA4(MCF_EPORT_EPPAR_FALLING) +
			MCF_EPORT_EPPAR_EPPA3(MCF_EPORT_EPPAR_FALLING) +
			MCF_EPORT_EPPAR_EPPA2(MCF_EPORT_EPPAR_FALLING) +
			MCF_EPORT_EPPAR_EPPA1(MCF_EPORT_EPPAR_FALLING);
	MCF_EPORT_EPDDR = 0;	/* clear data direction register. All pins as input */
	MCF_EPORT_EPFR = 0;		/* clear all EPORT interrupt flags */
	MCF_EPORT_EPIER = 0;	/* disable all EPORT interrupts (for now) */
}

void init_xlbus_arbiter(void)
{
	uint8_t clock_ratio;

	/* setup XL bus arbiter */
	clock_ratio = (MCF_PCI_PCIGSCR >> 24) & 0x07;

	if (clock_ratio == 4)
	{
		/* device errata 26: Flexbus hang up in 4:1 clock ratio */
		MCF_PCI_PCIGSCR |= 0x80000000; /* disable pipeline */
	}
	/* FIXME: Firetos (boot2.S, l. 719) looks pretty strange at this place - is this a typo? */
}

void init_pci(void)
{
	xprintf("initializing PCI bridge:");

   MCF_PCIARB_PACR = MCF_PCIARB_PACR_INTMPRI
       + MCF_PCIARB_PACR_EXTMPRI(0x1F)
       + MCF_PCIARB_PACR_INTMINTEN
       + MCF_PCIARB_PACR_EXTMINTEN(0x1F);

   /* Setup burst parameters */
   MCF_PCI_PCICR1 = MCF_PCI_PCICR1_CACHELINESIZE(4) + MCF_PCI_PCICR1_LATTIMER(32);
   MCF_PCI_PCICR2 = MCF_PCI_PCICR2_MINGNT(16) + MCF_PCI_PCICR2_MAXLAT(16);

   /* Turn on error signaling */
   MCF_PCI_PCIICR = MCF_PCI_PCIICR_TAE + MCF_PCI_PCIICR_TAE + MCF_PCI_PCIICR_REE + 32;
   MCF_PCI_PCIGSCR |= MCF_PCI_PCIGSCR_SEE;

   /* Configure Initiator Windows */
   /* initiator window 0 base / translation adress register */
   MCF_PCI_PCIIW0BTAR = (PCI_MEMORY_OFFSET + ((PCI_MEMORY_SIZE -1) >> 8)) & 0xffff0000;

   /* initiator window 1 base / translation adress register */
   MCF_PCI_PCIIW1BTAR = (PCI_IO_OFFSET + ((PCI_IO_SIZE - 1) >> 8)) & 0xffff0000;

   /* initiator window 2 base / translation address register */
   MCF_PCI_PCIIW2BTAR = 0L;   /* not used */

   /* initiator window configuration register */
   MCF_PCI_PCIIWCR = MCF_PCI_PCIIWCR_WINCTRL0_MEMRDLINE + MCF_PCI_PCIIWCR_WINCTRL1_IO;

   /* reset PCI devices */
   MCF_PCI_PCIGSCR &= ~MCF_PCI_PCIGSCR_PR;

   xprintf("finished\r\n");

	pci_scan();
}

