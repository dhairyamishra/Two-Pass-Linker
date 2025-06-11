#!/bin/bash

DIR1=$1
DIR2=$2
LOG=${3:-${DIR2}/LOG.txt}

INS="`seq 1 20`"

OUTPRE="out-"
INPRE="input-"

DIFFCMD="diff -b -B"
CHKSUM="md5sum"

[[ ${#} -lt 2 ]] && echo "usage: $0 <refoutdir> <youroutdir>" && exit
[[ ! -d ${DIR1} ]] && echo "refoutdir <$DIR1> does not exist"  && exit
[[ ! -d ${DIR2} ]]  && echo "youroutdir <$DIR2> does not exist"  && exit

############################################################################
#  NO TRACING 
############################################################################

declare -ai counters
declare -i x=0
declare -i correct=0

declare -i COUNT=0
rm -f ${LOG}

X1=`id -un`;X2=`id -ua`;X3=`hostname -s`;X4=$(date +"%m%d/%H%M");X5=`gcc -v 2>&1 | grep "^gcc" | cut -d' ' -f3`;X6="/home/${X1}/${X2}/${X3}/${X4}/${X5}";X7=`echo ${X6} | ${CHKSUM}`;echo "${X7} ${X6}";echo;

for f in ${INS}; do
    OUTLINE=`printf "%02d" ${f}`
    COUNT=0
    OUTF="${OUTPRE}${f}"
    if [[ ! -e ${DIR1}/${OUTF} ]]; then
        echo "${DIR1}/${OUTF} does not exist" >> ${LOG}
    elif [[ ! -e ${DIR2}/${OUTF} ]]; then
        echo "${DIR2}/${OUTF} does not exist" >> ${LOG}
    else
    
        #  echo "${DIFFCMD} ${DARGS} ${DIR1}/${OUTF} ${DIR2}/${OUTF}"
        #######################################################
        #
        # parse errors there should only be one and its the first one
        # to be printed. That will be caught by the -v diff
    
        # if [[ `wc -l  | cut -d' ' -f 1` == 1 ]]; then echo        
        # DIFF=`${DIFFCMD} ${DARGS} ${DIR1}/${OUTF} ${DIR2}/${OUTF}`

        ##################################################################  
        ${DIFFCMD} ${DARGS} ${DIR1}/${OUTF} ${DIR2}/${OUTF} 2>&1 > /dev/null
        if [[ $? -eq 0 ]]; then                                            
            COUNT=`expr ${COUNT} + 1`
        else
            echo "################### ${INPRE}${f}##################" >> ${LOG}
            ${DIFFCMD} ${DARGS} ${DIR1}/${OUTF} ${DIR2}/${OUTF} 2>&1 >> ${LOG}
        fi
    fi

    OUTLINE=`printf "%s %1d" "${OUTLINE}" "${COUNT}"`
#   echo `expr ${counters[$x]} + ${COUNT}`
    let counters[$x]=`expr ${counters[$x]}+${COUNT}`
    let x=$x+1
    let correct=$correct+${COUNT}
done

OUTLINE="RES $correct of $x: "
x=0
for s in ${INS}; do 
	OUTLINE=`printf "%s %d" "${OUTLINE}" "${counters[$x]}"`
	let x=$x+1
done
echo "${OUTLINE}"

PTS=`echo "scale=4; 60.0*(${correct}.0 / ${x}.0)" | bc`
RESULT=`echo "scale=4; 40+${PTS}" | bc`

echo
echo "LABRESULTS = 40 + ${PTS} - DEDUCTION = $RESULT"

