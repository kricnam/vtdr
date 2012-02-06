/******************** (C) COPYRIGHT 2012 M ********************
 * File Name          : atmel_dataflash.c
 * Author             : MXX
 * Description        : atmel data flash driver source file.
 *                      Pin assignment:
 *             ----------------------------------------------
 *             |  STM32F10x    |     DATAFLASH    Pin        |
 *             ----------------------------------------------
 *             | PA.4          |   ChipSelect      1         |
 *             | PA.6 / MISO   |   DataOut         2         |
 *             |               |   WP              3 (3.3 V) |
 *             |               |   GND             4 (0 V)   |
 *             | PA.7 / MOSI   |   DataIn          5         |
 *             | PA.5 / SCLK   |   Clock           6         |
 *             |               |   Hold            7 (3.3V)  |
 *             |               |   VDD             8         |
 *             -----------------------------------------------
 ********************************************************************************
 * THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
 * WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE TIME.
 * AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY DIRECT,
 * INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING FROM THE
 * CONTENT OF SUCH SOFTWARE AND/OR THE USE MADE BY CUSTOMERS OF THE CODING
 * INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
 * FOR MORE INFORMATION PLEASE CAREFULLY READ THE LICENSE AGREEMENT FILE LOCATED
 * IN THE ROOT DIRECTORY OF THIS FIRMWARE PACKAGE.
 *******************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "atmel_dataflash.h"
#include <stm32f10x_spi.h>
#include <stm32f10x_gpio.h>
#include <stm32f10x_rcc.h>
/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Select DATAFLASH Card: ChipSelect pin low  */
#define DATAFLASH_CS_LOW()    GPIO_ResetBits(GPIOA, GPIO_Pin_4)
/* Deselect DATAFLASH Card: ChipSelect pin high */
#define DATAFLASH_CS_HIGH()   GPIO_SetBits(GPIOA, GPIO_Pin_4)
#define DATAFLASH_SPI         SPI1
#define DATAFLASH_RCC_SPI     RCC_APB2Periph_SPI1
#define DATAFLASH_Manufacturer_ATMEL 0x1F
#define DATAFLASH_READSUCCESS(x)  (((x>>8) & 0x00FF)!=0)
#define DATAFLASH_READ_BYTE(x)    (u8)(x&0x00FF)
/* Private function prototypes -----------------------------------------------*/
static void SPI_Config(void);
/* Private functions ---------------------------------------------------------*/

void dataflash_delay(u16 n)
{
	for(u16 i=0;i<n;i++);
}
/*******************************************************************************
 * Function Name  : DATAFLASH_Init
 * Description    : Initializes the DATAFLASH/SD communication.
 * Input          : None
 * Output         : None
 * Return         : The DATAFLASH Response: - DATAFLASH_RESPONSE_FAILURE: Sequence failed
 *                                    - DATAFLASH_RESPONSE_NO_ERROR: Sequence succeed
 *******************************************************************************/
u8 DATAFLASH_Init(void)
{
        /* Initialize SPI */
	SPI_Config();
	/* DATAFLASH chip select high */
	DATAFLASH_CS_HIGH();
	/* Send dummy byte 0xFF, 10 times with CS high*/
	/* rise CS and MOSI for 80 clocks cycles */

	/*------------Put DATAFLASH in SPI mode--------------*/
	/* DATAFLASH initialized and set to SPI mode properly */
	sDATAFLASH_CID cid;
	return (DATAFLASH_GetDeviceID(&cid));
}

/*******************************************************************************
 * Function Name  : DATAFLASH_WriteBlock
 * Description    : Writes a block on the DATAFLASH
 * Input          : - pBuffer : pointer to the buffer containing the data to be
 *                    written on the DATAFLASH.
 *                  - WriteAddr : address to write on.
 *                  - NumByteToWrite: number of data to write
 * Output         : None
 * Return         : The DATAFLASH Response: - DATAFLASH_RESPONSE_FAILURE: Sequence failed
 *                                    - DATAFLASH_RESPONSE_NO_ERROR: Sequence succeed
 *******************************************************************************/
u8 DATAFLASH_WriteBlock(u8* pBuffer, u32 WriteAddr, u16 NumByteToWrite)
{
	u32 i = 0;
	u8 rvalue = DATAFLASH_RESPONSE_FAILURE;

	/* DATAFLASH chip select low */
	DATAFLASH_CS_LOW();
	/* Send CMD24 (DATAFLASH_WRITE_BLOCK) to write multiple block */
	DATAFLASH_SendCmd(DATAFLASH_WRITE_BLOCK, WriteAddr, 0xFF);

	/* Check if the DATAFLASH acknowledged the write block command: R1 response (0x00: no errors) */
	if (!DATAFLASH_GetResponse(DATAFLASH_RESPONSE_NO_ERROR))
	{
		/* Send a dummy byte */
		DATAFLASH_WriteByte(DUMMY);
		/* Send the data token to signify the start of the data */
		DATAFLASH_WriteByte(0xFE);
		/* Write the block data to DATAFLASH : write count data by block */
		for (i = 0; i < NumByteToWrite; i++)
		{
			/* Send the pointed byte */
			DATAFLASH_WriteByte(*pBuffer);
			/* Point to the next location where the byte read will be saved */
			pBuffer++;
		}
		/* Put CRC bytes (not really needed by us, but required by DATAFLASH) */
		DATAFLASH_ReadByte();
		DATAFLASH_ReadByte();
		/* Read data response */
		if (DATAFLASH_GetDataResponse() == DATAFLASH_DATA_OK)
		{
			rvalue = DATAFLASH_RESPONSE_NO_ERROR;
		}
	}

	/* DATAFLASH chip select high */
	DATAFLASH_CS_HIGH();
	/* Send dummy byte: 8 Clock pulses of delay */
	DATAFLASH_WriteByte(DUMMY);
	/* Returns the reponse */
	return rvalue;
}

/*******************************************************************************
 * Function Name  : DATAFLASH_ReadBlock
 * Description    : Reads a block of data from the DATAFLASH.
 * Input          : - pBuffer : pointer to the buffer that receives the data read
 *                    from the DATAFLASH.
 *                  - ReadAddr : DATAFLASH's internal address to read from.
 *                  - NumByteToRead : number of bytes to read from the DATAFLASH.
 * Output         : None
 * Return         : The DATAFLASH Response: - DATAFLASH_RESPONSE_FAILURE: Sequence failed
 *                                    - DATAFLASH_RESPONSE_NO_ERROR: Sequence succeed
 *******************************************************************************/
u8 DATAFLASH_ReadBlock(u8* pBuffer, u32 ReadAddr, u16 NumByteToRead)
{
	u32 i = 0;
	u8 rvalue = DATAFLASH_RESPONSE_FAILURE;

	/* DATAFLASH chip select low */
	DATAFLASH_CS_LOW();
	/* Send CMD17 (DATAFLASH_READ_SINGLE_BLOCK) to read one block */
	DATAFLASH_SendCmd(DATAFLASH_READ_SINGLE_BLOCK, ReadAddr, 0xFF);

	/* Check if the DATAFLASH acknowledged the read block command: R1 response (0x00: no errors) */
	if (!DATAFLASH_GetResponse(DATAFLASH_RESPONSE_NO_ERROR))
	{
		/* Now look for the data token to signify the start of the data */
		if (!DATAFLASH_GetResponse(DATAFLASH_START_DATA_SINGLE_BLOCK_READ))
		{
			/* Read the DATAFLASH block data : read NumByteToRead data */
			for (i = 0; i < NumByteToRead; i++)
			{
				/* Save the received data */
				*pBuffer = DATAFLASH_ReadByte();
				/* Point to the next location where the byte read will be saved */
				pBuffer++;
			}
			/* Get CRC bytes (not really needed by us, but required by DATAFLASH) */
			DATAFLASH_ReadByte();
			DATAFLASH_ReadByte();
			/* Set response value to success */
			rvalue = DATAFLASH_RESPONSE_NO_ERROR;
		}
	}

	/* DATAFLASH chip select high */
	DATAFLASH_CS_HIGH();
	/* Send dummy byte: 8 Clock pulses of delay */
	DATAFLASH_WriteByte(DUMMY);
	/* Returns the reponse */
	return rvalue;
}

/*******************************************************************************
 * Function Name  : DATAFLASH_WriteBuffer
 * Description    : Writes many blocks on the DATAFLASH
 * Input          : - pBuffer : pointer to the buffer containing the data to be
 *                    written on the DATAFLASH.
 *                  - WriteAddr : address to write on.
 *                  - NumByteToWrite: number of data to write
 * Output         : None
 * Return         : The DATAFLASH Response: - DATAFLASH_RESPONSE_FAILURE: Sequence failed
 *                                    - DATAFLASH_RESPONSE_NO_ERROR: Sequence succeed
 *******************************************************************************/
u8 DATAFLASH_WriteBuffer(u8* pBuffer, u32 WriteAddr, u32 NumByteToWrite)
{
	u32 i = 0, NbrOfBlock = 0, Offset = 0;
	u8 rvalue = DATAFLASH_RESPONSE_FAILURE;

	/* Calculate number of blocks to write */
	NbrOfBlock = NumByteToWrite / BLOCK_SIZE;
	/* DATAFLASH chip select low */
	DATAFLASH_CS_LOW();

	/* Data transfer */
	while (NbrOfBlock--)
	{
		/* Send CMD24 (DATAFLASH_WRITE_BLOCK) to write blocks */
		DATAFLASH_SendCmd(DATAFLASH_WRITE_BLOCK, WriteAddr + Offset, 0xFF);

		/* Check if the DATAFLASH acknowledged the write block command: R1 response (0x00: no errors) */
		if (DATAFLASH_GetResponse(DATAFLASH_RESPONSE_NO_ERROR))
		{
			return DATAFLASH_RESPONSE_FAILURE;
		}
		/* Send dummy byte */
		DATAFLASH_WriteByte(DUMMY);
		/* Send the data token to signify the start of the data */
		DATAFLASH_WriteByte(DATAFLASH_START_DATA_SINGLE_BLOCK_WRITE);
		/* Write the block data to DATAFLASH : write count data by block */
		for (i = 0; i < BLOCK_SIZE; i++)
		{
			/* Send the pointed byte */
			DATAFLASH_WriteByte(*pBuffer);
			/* Point to the next location where the byte read will be saved */
			pBuffer++;
		}
		/* Set next write address */
		Offset += 512;
		/* Put CRC bytes (not really needed by us, but required by DATAFLASH) */
		DATAFLASH_ReadByte();
		DATAFLASH_ReadByte();
		/* Read data response */
		if (DATAFLASH_GetDataResponse() == DATAFLASH_DATA_OK)
		{
			/* Set response value to success */
			rvalue = DATAFLASH_RESPONSE_NO_ERROR;
		}
		else
		{
			/* Set response value to failure */
			rvalue = DATAFLASH_RESPONSE_FAILURE;
		}
	}

	/* DATAFLASH chip select high */
	DATAFLASH_CS_HIGH();
	/* Send dummy byte: 8 Clock pulses of delay */
	DATAFLASH_WriteByte(DUMMY);
	/* Returns the reponse */
	return rvalue;
}

/*******************************************************************************
 * Function Name  : DATAFLASH_ReadBuffer
 * Description    : Reads multiple block of data from the DATAFLASH.
 * Input          : - pBuffer : pointer to the buffer that receives the data read
 *                    from the DATAFLASH.
 *                  - ReadAddr : DATAFLASH's internal address to read from.
 *                  - NumByteToRead : number of bytes to read from the DATAFLASH.
 * Output         : None
 * Return         : The DATAFLASH Response: - DATAFLASH_RESPONSE_FAILURE: Sequence failed
 *                                    - DATAFLASH_RESPONSE_NO_ERROR: Sequence succeed
 *******************************************************************************/
u8 DATAFLASH_ReadBuffer(u8* pBuffer, u32 ReadAddr, u32 NumByteToRead)
{
	u32 i = 0, NbrOfBlock = 0, Offset = 0;
	u8 rvalue = DATAFLASH_RESPONSE_FAILURE;

	/* Calculate number of blocks to read */
	NbrOfBlock = NumByteToRead / BLOCK_SIZE;
	/* DATAFLASH chip select low */
	DATAFLASH_CS_LOW();

	/* Data transfer */
	while (NbrOfBlock--)
	{
		/* Send CMD17 (DATAFLASH_READ_SINGLE_BLOCK) to read one block */
		DATAFLASH_SendCmd(DATAFLASH_READ_SINGLE_BLOCK, ReadAddr + Offset, 0xFF);
		/* Check if the DATAFLASH acknowledged the read block command: R1 response (0x00: no errors) */
		if (DATAFLASH_GetResponse(DATAFLASH_RESPONSE_NO_ERROR))
		{
			return DATAFLASH_RESPONSE_FAILURE;
		}
		/* Now look for the data token to signify the start of the data */
		if (!DATAFLASH_GetResponse(DATAFLASH_START_DATA_SINGLE_BLOCK_READ))
		{
			/* Read the DATAFLASH block data : read NumByteToRead data */
			for (i = 0; i < BLOCK_SIZE; i++)
			{
				/* Read the pointed data */
				*pBuffer = DATAFLASH_ReadByte();
				/* Point to the next location where the byte read will be saved */
				pBuffer++;
			}
			/* Set next read address*/
			Offset += 512;
			/* get CRC bytes (not really needed by us, but required by DATAFLASH) */
			DATAFLASH_ReadByte();
			DATAFLASH_ReadByte();
			/* Set response value to success */
			rvalue = DATAFLASH_RESPONSE_NO_ERROR;
		}
		else
		{
			/* Set response value to failure */
			rvalue = DATAFLASH_RESPONSE_FAILURE;
		}
	}

	/* DATAFLASH chip select high */
	DATAFLASH_CS_HIGH();
	/* Send dummy byte: 8 Clock pulses of delay */
	DATAFLASH_WriteByte(DUMMY);
	/* Returns the reponse */
	return rvalue;
}

/*******************************************************************************
 * Function Name  : DATAFLASH_GetCSDRegister
 * Description    : Read the CSD card register.
 *                  Reading the contents of the CSD register in SPI mode
 *                  is a simple read-block transaction.
 * Input          : - DATAFLASH_csd: pointer on an SCD register structure
 * Output         : None
 * Return         : The DATAFLASH Response: - DATAFLASH_RESPONSE_FAILURE: Sequence failed
 *                                    - DATAFLASH_RESPONSE_NO_ERROR: Sequence succeed
 *******************************************************************************/
u8 DATAFLASH_GetCSDRegister(sDATAFLASH_CSD* DATAFLASH_csd)
{
	u32 i = 0;
	u8 rvalue = DATAFLASH_RESPONSE_FAILURE;
	u8 CSD_Tab[16];

	/* DATAFLASH chip select low */
	DATAFLASH_CS_LOW();
	/* Send CMD9 (CSD register) or CMD10(CSD register) */
	DATAFLASH_SendCmd(DATAFLASH_SEND_CSD, 0, 0xFF);

	/* Wait for response in the R1 format (0x00 is no errors) */
	if (!DATAFLASH_GetResponse(DATAFLASH_RESPONSE_NO_ERROR))
	{
		if (!DATAFLASH_GetResponse(DATAFLASH_START_DATA_SINGLE_BLOCK_READ))
		{
			for (i = 0; i < 16; i++)
			{
				/* Store CSD register value on CSD_Tab */
				CSD_Tab[i] = DATAFLASH_ReadByte();
			}
		}
		/* Get CRC bytes (not really needed by us, but required by DATAFLASH) */
		DATAFLASH_WriteByte(DUMMY);
		DATAFLASH_WriteByte(DUMMY);
		/* Set response value to success */
		rvalue = DATAFLASH_RESPONSE_NO_ERROR;
	}

	/* DATAFLASH chip select high */
	DATAFLASH_CS_HIGH();
	/* Send dummy byte: 8 Clock pulses of delay */
	DATAFLASH_WriteByte(DUMMY);

	/* Byte 0 */
	DATAFLASH_csd->CSDStruct = (CSD_Tab[0] & 0xC0) >> 6;
	DATAFLASH_csd->SysSpecVersion = (CSD_Tab[0] & 0x3C) >> 2;
	DATAFLASH_csd->Reserved1 = CSD_Tab[0] & 0x03;
	/* Byte 1 */
	DATAFLASH_csd->TAAC = CSD_Tab[1];
	/* Byte 2 */
	DATAFLASH_csd->NSAC = CSD_Tab[2];
	/* Byte 3 */
	DATAFLASH_csd->MaxBusClkFrec = CSD_Tab[3];
	/* Byte 4 */
	DATAFLASH_csd->CardComdClasses = CSD_Tab[4] << 4;
	/* Byte 5 */
	DATAFLASH_csd->CardComdClasses |= (CSD_Tab[5] & 0xF0) >> 4;
	DATAFLASH_csd->RdBlockLen = CSD_Tab[5] & 0x0F;
	/* Byte 6 */
	DATAFLASH_csd->PartBlockRead = (CSD_Tab[6] & 0x80) >> 7;
	DATAFLASH_csd->WrBlockMisalign = (CSD_Tab[6] & 0x40) >> 6;
	DATAFLASH_csd->RdBlockMisalign = (CSD_Tab[6] & 0x20) >> 5;
	DATAFLASH_csd->DSRImpl = (CSD_Tab[6] & 0x10) >> 4;
	DATAFLASH_csd->Reserved2 = 0; /* Reserved */
	DATAFLASH_csd->DeviceSize = (CSD_Tab[6] & 0x03) << 10;
	/* Byte 7 */
	DATAFLASH_csd->DeviceSize |= (CSD_Tab[7]) << 2;
	/* Byte 8 */
	DATAFLASH_csd->DeviceSize |= (CSD_Tab[8] & 0xC0) >> 6;
	DATAFLASH_csd->MaxRdCurrentVDDMin = (CSD_Tab[8] & 0x38) >> 3;
	DATAFLASH_csd->MaxRdCurrentVDDMax = (CSD_Tab[8] & 0x07);
	/* Byte 9 */
	DATAFLASH_csd->MaxWrCurrentVDDMin = (CSD_Tab[9] & 0xE0) >> 5;
	DATAFLASH_csd->MaxWrCurrentVDDMax = (CSD_Tab[9] & 0x1C) >> 2;
	DATAFLASH_csd->DeviceSizeMul = (CSD_Tab[9] & 0x03) << 1;
	/* Byte 10 */
	DATAFLASH_csd->DeviceSizeMul |= (CSD_Tab[10] & 0x80) >> 7;
	DATAFLASH_csd->EraseGrSize = (CSD_Tab[10] & 0x7C) >> 2;
	DATAFLASH_csd->EraseGrMul = (CSD_Tab[10] & 0x03) << 3;
	/* Byte 11 */
	DATAFLASH_csd->EraseGrMul |= (CSD_Tab[11] & 0xE0) >> 5;
	DATAFLASH_csd->WrProtectGrSize = (CSD_Tab[11] & 0x1F);
	/* Byte 12 */
	DATAFLASH_csd->WrProtectGrEnable = (CSD_Tab[12] & 0x80) >> 7;
	DATAFLASH_csd->ManDeflECC = (CSD_Tab[12] & 0x60) >> 5;
	DATAFLASH_csd->WrSpeedFact = (CSD_Tab[12] & 0x1C) >> 2;
	DATAFLASH_csd->MaxWrBlockLen = (CSD_Tab[12] & 0x03) << 2;
	/* Byte 13 */
	DATAFLASH_csd->MaxWrBlockLen |= (CSD_Tab[13] & 0xc0) >> 6;
	DATAFLASH_csd->WriteBlockPaPartial = (CSD_Tab[13] & 0x20) >> 5;
	DATAFLASH_csd->Reserved3 = 0;
	DATAFLASH_csd->ContentProtectAppli = (CSD_Tab[13] & 0x01);
	/* Byte 14 */
	DATAFLASH_csd->FileFormatGrouop = (CSD_Tab[14] & 0x80) >> 7;
	DATAFLASH_csd->CopyFlag = (CSD_Tab[14] & 0x40) >> 6;
	DATAFLASH_csd->PermWrProtect = (CSD_Tab[14] & 0x20) >> 5;
	DATAFLASH_csd->TempWrProtect = (CSD_Tab[14] & 0x10) >> 4;
	DATAFLASH_csd->FileFormat = (CSD_Tab[14] & 0x0C) >> 2;
	DATAFLASH_csd->ECC = (CSD_Tab[14] & 0x03);
	/* Byte 15 */
	DATAFLASH_csd->msd_CRC = (CSD_Tab[15] & 0xFE) >> 1;
	DATAFLASH_csd->Reserved4 = 1;

	/* Return the reponse */
	return rvalue;
}

/*******************************************************************************
 * Function Name  : DATAFLASH_GetCIDRegister
 * Description    : Read the CID card register.
 *                  Reading the contents of the CID register in SPI mode
 *                  is a simple read-block transaction.
 * Input          : - DATAFLASH_cid: pointer on an CID register structure
 * Output         : None
 * Return         : The DATAFLASH Response: - DATAFLASH_RESPONSE_FAILURE: Sequence failed
 *                                    - DATAFLASH_RESPONSE_NO_ERROR: Sequence succeed
 *******************************************************************************/
u8 DATAFLASH_GetDeviceID(sDATAFLASH_CID* DATAFLASH_cid)
{
	u8 rvalue = DATAFLASH_RESPONSE_FAILURE;
	u8 CID_Tab[4];

	DATAFLASH_CS_HIGH();
	dataflash_delay(2000);
	/* DATAFLASH chip select low */
	DATAFLASH_CS_LOW();
	/* Send CMD10 (CID register) */
	dataflash_delay(1000);
	DATAFLASH_WriteByte(DATAFLASH_READ_CID);
	DATAFLASH_ReadByte();
	/* Store CID register value on CID_Tab */
	/* Wait until a data is received */
	for (int i=0;i < 4;i++)
	{
		
DATAFLASH_WriteByte(0xFF);
		u16 data = DATAFLASH_ReadByte();
		if (DATAFLASH_READSUCCESS(data))
		{
			CID_Tab[i] = DATAFLASH_READ_BYTE(data);
		}
		else
		{
			rvalue = DATAFLASH_RESPONSE_FAILURE;
			break;
		}
		rvalue = DATAFLASH_RESPONSE_NO_ERROR;
                
	}

	/* DATAFLASH chip select high */
	DATAFLASH_CS_HIGH();
	/* Byte 0 */
	DATAFLASH_cid->ManufacturerID = CID_Tab[0];
	/* Byte 1 */
	u8* p = (u8*) &(DATAFLASH_cid->ManufacturerID);
	p[1] = CID_Tab[1];
	/* Byte 2 */
	p[2] = CID_Tab[2];

	/* Return the reponse */
	return rvalue;
}

/*******************************************************************************
 * Function Name  : DATAFLASH_SendCmd
 * Description    : Send 5 bytes command to the DATAFLASH card.
 * Input          : - Cmd: the user expected command to send to DATAFLASH card
 *                  - Arg: the command argument
 *                  - Crc: the CRC
 * Output         : None
 * Return         : None
 *******************************************************************************/
void DATAFLASH_SendCmd(u8 Cmd, u32 Arg, u8 Crc)
{
	u32 i = 0x00;
	u8 Frame[6];

	/* Construct byte1 */
	Frame[0] = (Cmd | 0x40);
	/* Construct byte2 */
	Frame[1] = (u8) (Arg >> 24);
	/* Construct byte3 */
	Frame[2] = (u8) (Arg >> 16);
	/* Construct byte4 */
	Frame[3] = (u8) (Arg >> 8);
	/* Construct byte5 */
	Frame[4] = (u8) (Arg);
	/* Construct CRC: byte6 */
	Frame[5] = (Crc);

	/* Send the Cmd bytes */
	for (i = 0; i < 6; i++)
	{
		DATAFLASH_WriteByte(Frame[i]);
	}
}

/*******************************************************************************
 * Function Name  : DATAFLASH_GetDataResponse
 * Description    : Get DATAFLASH card data response.
 * Input          : None
 * Output         : None
 * Return         : The DATAFLASH status: Read data response xxx0<status>1
 *                   - status 010: Data accecpted
 *                   - status 101: Data rejected due to a crc error
 *                   - status 110: Data rejected due to a Write error.
 *                   - status 111: Data rejected due to other error.
 *******************************************************************************/
u8 DATAFLASH_GetDataResponse(void)
{
	u32 i = 0;
	u8 response, rvalue;

	while (i <= 64)
	{
		/* Read resonse */
		response = DATAFLASH_ReadByte();
		/* Mask unused bits */
		response &= 0x1F;

		switch (response)
		{
		case DATAFLASH_DATA_OK:
		{
			rvalue = DATAFLASH_DATA_OK;
			break;
		}

		case DATAFLASH_DATA_CRC_ERROR:
			return DATAFLASH_DATA_CRC_ERROR;

		case DATAFLASH_DATA_WRITE_ERROR:
			return DATAFLASH_DATA_WRITE_ERROR;

		default:
		{
			rvalue = DATAFLASH_DATA_OTHER_ERROR;
			break;
		}
		}
		/* Exit loop in case of data ok */
		if (rvalue == DATAFLASH_DATA_OK)
			break;
		/* Increment loop counter */
		i++;
	}
	/* Wait null data */
	while (DATAFLASH_ReadByte() == 0)
		;
	/* Return response */
	return response;
}

/*******************************************************************************
 * Function Name  : DATAFLASH_GetResponse
 * Description    : Returns the DATAFLASH response.
 * Input          : None
 * Output         : None
 * Return         : The DATAFLASH Response: - DATAFLASH_RESPONSE_FAILURE: Sequence failed
 *                                    - DATAFLASH_RESPONSE_NO_ERROR: Sequence succeed
 *******************************************************************************/
u8 DATAFLASH_GetResponse(u8 Response)
{
	u32 Count = 0xFFF;

	/* Check if response is got or a timeout is happen */
	while ((DATAFLASH_ReadByte() != Response) && Count)
	{
		Count--;
	}

	if (Count == 0)
	{
		/* After time out */
		return DATAFLASH_RESPONSE_FAILURE;
	}
	else
	{
		/* Right response got */
		return DATAFLASH_RESPONSE_NO_ERROR;
	}
}

/*******************************************************************************
 * Function Name  : DATAFLASH_GetStatus
 * Description    : Returns the DATAFLASH status.
 * Input          : None
 * Output         : None
 * Return         : The DATAFLASH status.
 *******************************************************************************/
u16 DATAFLASH_GetStatus(void)
{
	u16 Status = 0;

	/* DATAFLASH chip select low */
	DATAFLASH_CS_LOW();
	/* Send CMD13 (DATAFLASH_SEND_STATUS) to get DATAFLASH status */
	DATAFLASH_SendCmd(DATAFLASH_SEND_STATUS, 0, 0xFF);

	Status = DATAFLASH_ReadByte();
	Status |= (u16) (DATAFLASH_ReadByte() << 8);

	/* DATAFLASH chip select high */
	DATAFLASH_CS_HIGH();
	/* Send dummy byte 0xFF */
	DATAFLASH_WriteByte(DUMMY);

	return Status;
}

/*******************************************************************************
 * Function Name  : DATAFLASH_GoIdleState
 * Description    : Put DATAFLASH in Idle state.
 * Input          : None
 * Output         : None
 * Return         : The DATAFLASH Response: - DATAFLASH_RESPONSE_FAILURE: Sequence failed
 *                                    - DATAFLASH_RESPONSE_NO_ERROR: Sequence succeed
 *******************************************************************************/
u8 DATAFLASH_GoIdleState(void)
{
	DATAFLASH_CS_LOW();

	/* DATAFLASH chip select low */
	DATAFLASH_CS_LOW();
	/* Send CMD0 (GO_IDLE_STATE) to put DATAFLASH in SPI mode */
	DATAFLASH_SendCmd(DATAFLASH_GO_IDLE_STATE, 0, 0x95);

	/* Wait for In Idle State Response (R1 Format) equal to 0x01 */
	if (DATAFLASH_GetResponse(DATAFLASH_IN_IDLE_STATE))
	{
		/* No Idle State Response: return response failue */
		return DATAFLASH_RESPONSE_FAILURE;
	}
	/*----------Activates the card initialization process-----------*/
	do
	{
		/* DATAFLASH chip select high */
		DATAFLASH_CS_HIGH();
		/* Send Dummy byte 0xFF */
		DATAFLASH_WriteByte(DUMMY);

		/* DATAFLASH chip select low */
		DATAFLASH_CS_LOW();

		/* Send CMD1 (Activates the card process) until response equal to 0x0 */
		DATAFLASH_SendCmd(DATAFLASH_SEND_OP_COND, 0, 0xFF);
		/* Wait for no error Response (R1 Format) equal to 0x00 */
	} while (DATAFLASH_GetResponse(DATAFLASH_RESPONSE_NO_ERROR));

	/* DATAFLASH chip select high */
	DATAFLASH_CS_HIGH();
	/* Send dummy byte 0xFF */
	DATAFLASH_WriteByte(DUMMY);

	return DATAFLASH_RESPONSE_NO_ERROR;
}

/*******************************************************************************
 * Function Name  : DATAFLASH_WriteByte
 * Description    : Write a byte on the DATAFLASH.
 * Input          : Data: byte to send.
 * Output         : None
 * Return         : None.
 *******************************************************************************/
void DATAFLASH_WriteByte(u8 Data)
{
	/* Wait until the transmit buffer is empty */
	while (SPI_I2S_GetFlagStatus(DATAFLASH_SPI, SPI_I2S_FLAG_TXE) == RESET)
		;
	/* Send the byte */
	SPI_I2S_SendData(DATAFLASH_SPI, Data);
}

/*******************************************************************************
 * Function Name  : DATAFLASH_ReadByte
 * Description    : Read a byte from the DATAFLASH.
 * Input          : None.
 * Output         : None
 * Return         : The received byte.
 *******************************************************************************/
u16 DATAFLASH_ReadByte(void)
{
	u8 Data = 0;
	u8 count=0xFF;
	/* Wait until a data is received */
	while ((--count) && SPI_I2S_GetFlagStatus(DATAFLASH_SPI, SPI_I2S_FLAG_RXNE) == RESET)
		;
	/* Get the received data */
	Data = SPI_I2S_ReceiveData(DATAFLASH_SPI);

	/* Return the shifted data */
	return ((count << 8)&0xFF00) | (0x00FF & Data);
}

/*******************************************************************************
 * Function Name  : SPI_Config
 * Description    : Initializes the SPI and CS pins.
 * Input          : None
 * Output         : None
 * Return         : None
 *******************************************************************************/
void SPI_Config(void)
{
	uint32_t delay;
	GPIO_InitTypeDef GPIO_InitStructure;
	SPI_InitTypeDef SPI_InitStructure;

	/* GPIOA and GPIOC Periph clock enable */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
	/* SPI Periph clock enable */
	RCC_APB2PeriphClockCmd(DATAFLASH_RCC_SPI, ENABLE);

	/* Configure SPI pins: SCK, MISO and MOSI */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_OD;
	GPIO_Init(GPIOD, &GPIO_InitStructure);

	/* SPI Config */
	SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
	SPI_InitStructure.SPI_Mode = SPI_Mode_Master;
	SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;
	SPI_InitStructure.SPI_CPOL = SPI_CPOL_High;
	SPI_InitStructure.SPI_CPHA = SPI_CPHA_2Edge;
	SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;
	SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_2;
	SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;
	SPI_InitStructure.SPI_CRCPolynomial = 7;
	SPI_Init(DATAFLASH_SPI, &SPI_InitStructure);

	/* SPI enable */
	SPI_Cmd(DATAFLASH_SPI, ENABLE);

	for (delay = 0; delay < 0xfffff; delay++)
		;
}

/******************* (C) COPYRIGHT 2008 STMicroelectronics *****END OF FILE****/

/*
 * RT-Thread SD Card Driver
 * 20090417 Bernard
 */
#include <rtthread.h>
#include <dfs_fs.h>

static struct rt_device dataflash_device;
static struct dfs_partition part;
#define SECTOR_SIZE 512

/* RT-Thread Device Driver Interface */
static rt_err_t rt_dataflash_init(rt_device_t dev)
{

	return RT_EOK;
}

static rt_err_t rt_dataflash_open(rt_device_t dev, rt_uint16_t oflag)
{
	return RT_EOK;
}

static rt_err_t rt_dataflash_close(rt_device_t dev)
{
	return RT_EOK;
}

static rt_size_t rt_dataflash_read(rt_device_t dev, rt_off_t pos, void* buffer,
		rt_size_t size)
{
	rt_uint8_t status;
	rt_uint32_t i;

	status = DATAFLASH_RESPONSE_NO_ERROR;
	// rt_kprintf("read: 0x%x, size %d\n", pos, size);

	/* read all sectors */
	for (i = 0; i < size / SECTOR_SIZE; i++)
	{
		status = DATAFLASH_ReadBlock((rt_uint8_t*) ((rt_uint8_t*) buffer + i
				* SECTOR_SIZE), (part.offset + i) * SECTOR_SIZE + pos,
				SECTOR_SIZE);
		if (status != DATAFLASH_RESPONSE_NO_ERROR)
		{
			rt_kprintf("dataflash read failed\n");
			return 0;
		}
	}

	if (status == DATAFLASH_RESPONSE_NO_ERROR)
		return size;

	rt_kprintf("read failed: %d\n", status);
	return 0;
}

static rt_size_t rt_dataflash_write(rt_device_t dev, rt_off_t pos,
		const void* buffer, rt_size_t size)
{
	rt_uint8_t status;
	rt_uint32_t i;

	status = DATAFLASH_RESPONSE_NO_ERROR;
	// rt_kprintf("write: 0x%x, size %d\n", pos, size);

	/* read all sectors */
	for (i = 0; i < size / SECTOR_SIZE; i++)
	{
		status = DATAFLASH_WriteBuffer((rt_uint8_t*) ((rt_uint8_t*) buffer + i
				* SECTOR_SIZE), (part.offset + i) * SECTOR_SIZE + pos,
				SECTOR_SIZE);
		if (status != DATAFLASH_RESPONSE_NO_ERROR)
		{
			rt_kprintf("dataflash write failed\n");
			return 0;
		}
	}

	if (status == DATAFLASH_RESPONSE_NO_ERROR)
		return size;

	rt_kprintf("write failed: %d\n", status);
	return 0;
}

static rt_err_t rt_dataflash_control(rt_device_t dev, rt_uint8_t cmd,
		void *args)
{
	return RT_EOK;
}

void rt_hw_dataflash_init()
{
	if (DATAFLASH_Init() == DATAFLASH_RESPONSE_NO_ERROR)
	{
		rt_uint8_t status;
		rt_uint8_t *sector;

		/* register dataflash device */
		dataflash_device.init = rt_dataflash_init;
		dataflash_device.open = rt_dataflash_open;
		dataflash_device.close = rt_dataflash_close;
		dataflash_device.read = rt_dataflash_read;
		dataflash_device.write = rt_dataflash_write;
		dataflash_device.control = rt_dataflash_control;

		/* no private */
		dataflash_device.private = RT_NULL;
		/* get the first sector to read partition table */
		sector = (rt_uint8_t*) rt_malloc(512);
		if (sector == RT_NULL)
		{
			rt_kprintf("allocate partition sector buffer failed\n");
			return;
		}

		/* release sector buffer */
		rt_free(sector);

		rt_device_register(&dataflash_device, "sd0", RT_DEVICE_FLAG_RDWR
				| RT_DEVICE_FLAG_REMOVABLE | RT_DEVICE_FLAG_STANDALONE);
	}
	else
	{
		rt_kprintf("dataflash init failed\n");
	}
}
