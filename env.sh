TOOL_PATH=/home/andyman/dev/github/forks/rumpkernel/obj.evbarm/tooldir.Linux-3.13.0-40-generic-x86_64/bin
TOOL_PREFIX=arm--netbsdelf-eabi-

export BUILDRUMP_SH=/home/andyman/dev/github/forks/rumpkernel/buildrump.sh

export PATH=${PATH}:${TOOL_PATH}
export CC=${TOOL_PATH}/${TOOL_PREFIX}gcc
export AR=${TOOL_PATH}/${TOOL_PREFIX}ar
export NM=${TOOL_PATH}/${TOOL_PREFIX}nm
export OBJCOPY=${TOOL_PATH}/${TOOL_PREFIX}objcopy

export GDB=${TOOL_PATH}/${TOOL_PREFIX}gdb
