################################################################################
#####################   update flash partition table first  ####################
################################################################################
set(FLASH_PYTHON_SCRIPT   ${CMAKE_SOURCE_DIR}/tools/python_scripts/before_build_gcc.py)
set(FLASH_PART_CFG_JSON   ${CMAKE_SOURCE_DIR}/project/${USER_PROJECT}/cfg/flash_partition_cfg.json)
set(FLASH_PART_TABLE_FILE ${CMAKE_SOURCE_DIR}/project/${USER_PROJECT}/cfg/flash_partition_table.h)

execute_process(COMMAND python3 ${FLASH_PYTHON_SCRIPT} -p ${FLASH_PART_CFG_JSON} -o ${FLASH_PART_TABLE_FILE}
  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/project/${USER_PROJECT}/cfg
)

################################################################################
###########################   update linker script  ############################
################################################################################
set(REWRITE_LINKERSCRIPT  ${CMAKE_SOURCE_DIR}/tools/python_scripts/rewrite_ln882x_linker_script.py)
set(LINKER_PATH           ${CMAKE_SOURCE_DIR}/project/${USER_PROJECT}/gcc/ln882x.ld)

execute_process(COMMAND python ${REWRITE_LINKERSCRIPT} ${FLASH_PART_CFG_JSON} ${LINKER_PATH}
  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/project/${USER_PROJECT}/gcc
)

################################################################################
################################   make image  #################################
################################################################################
set(BIN_TARGET    ${EXECUTABLE_OUTPUT_PATH}/${PROJECT_NAME}.bin)
set(HEX_TARGET    ${EXECUTABLE_OUTPUT_PATH}/${PROJECT_NAME}.hex)
set(MAP_TARGET    ${EXECUTABLE_OUTPUT_PATH}/${PROJECT_NAME}.map)
set(ASM_TARGET    ${EXECUTABLE_OUTPUT_PATH}/${PROJECT_NAME}.diasm)
set(FLASH_TARGET  ${EXECUTABLE_OUTPUT_PATH}/flashimage.bin)

# post build:
# 1) *.elf --> *.bin
# 2) *.bin + boot_ln882x.bin --> flashimage.bin
add_custom_command(TARGET  ${TARGET_ELF_NAME}
        POST_BUILD
        COMMAND  ${CMAKE_OBJCOPY}  -O  ihex     ${EXECUTABLE_OUTPUT_PATH}/${TARGET_ELF_NAME}     ${HEX_TARGET}
        COMMAND  ${CMAKE_OBJCOPY}  -O  binary   ${EXECUTABLE_OUTPUT_PATH}/${TARGET_ELF_NAME}     ${BIN_TARGET}
        COMMAND  ${CMAKE_OBJDUMP}  -S           ${EXECUTABLE_OUTPUT_PATH}/${TARGET_ELF_NAME}  >  ${ASM_TARGET}
        COMMAND  ${CMAKE_SIZE}                  ${EXECUTABLE_OUTPUT_PATH}/${TARGET_ELF_NAME}
        #COMMAND  ${LN_MKIMAGE}     cmd_app      ${CMAKE_SOURCE_DIR}/lib/boot_ln882x.bin  ${BIN_TARGET}  ${ASM_TARGET}  ${FLASH_TARGET}  ${CMAKE_SOURCE_DIR}/project/${USER_PROJECT}/cfg/flash_partition_cfg.json  ver=1.1
        COMMAND  python3   ${LN_MKIMAGE}   --sdkroot_dir ${CMAKE_SOURCE_DIR}  --userproj_dir  ${CMAKE_CURRENT_LIST_DIR}/../ --buildout_dir  ${EXECUTABLE_OUTPUT_PATH}  --buildout_name  ${PROJECT_NAME} --output flashimage.bin  --ver_major=${VER_MAJOR} --ver_minor=${VER_MINOR}
	WORKING_DIRECTORY  ${EXECUTABLE_OUTPUT_PATH}
        COMMENT  ">>>>>>  Generating ${HEX_TARGET}, ${BIN_TARGET}, ${ASM_TARGET}, ${FLASH_TARGET} v${VER_MAJOR}.${VER_MINOR} <<<<<<"
)
