      SUBROUTINEPCLUB3(N,K,I1,I2,IBLKSK,NDI,NUI,NI,IJU
     *,JCOL,IPVT,IRSW,ROW)
      IMPLICITDOUBLEPRECISION(A-H,O-Z)
      INTEGERN,K,I1,I2,IBLKSK,NDI,NUI,NI,IJU(*),JCOL(*),
     *IRSW(*),IPVT(*)
      DOUBLEPRECISIONROW(*)
      INTEGERIERR,ITMAX,ITEST,KMAX,IFL,IPRE,PBLK,LUDONE,IBLK,
     *MNEED,ITS,RSDONE,NOVERF,RED1,RED2,IERR2,IFRMAT
      COMMON/PCCOM1/IERR,ITMAX,ITEST,KMAX,IFL,IPRE,PBLK,
     *LUDONE,IBLK,MNEED,ITS,RSDONE,NOVERF,RED1,RED2,IERR2,
     *IFRMAT
      SAVE/PCCOM1/
      INTEGERII,I,IPVTI,RBASE,J,IPVTJ,RBASEJ,NIS,NIN
      DOUBLEPRECISIONT
      II=I1
      DO1I=1,IBLKSK
      IPVT(I)=I
      IRSW(II)=II
      II=II+1
1     CONTINUE
      II=I1-1
      NIN=NI-NDI
      DO3I=1,IBLKSK-1
      II=II+1
      IPVTI=IPVT(I)
      RBASE=(IPVTI-1)*N
      DO5J=I+1,IBLKSK
      IPVTJ=IPVT(J)
      RBASEJ=(IPVTJ-1)*N
      IF((ABS(ROW(RBASEJ+II)).GT.ABS(ROW(RBASE+II))).AND.(ROW
     *(RBASE+I1-1+IPVTJ).NE.0.0D0))THEN
      IPVT(J)=IPVTI
      IPVT(I)=IPVTJ
      IPVTI=IPVTJ
      RBASE=RBASEJ
      ENDIF
5     CONTINUE
      T=ROW(RBASE+II)
      IPVTI=IPVT(I)
      IF(T.EQ.0.0D0)THEN
      IERR=5
      IERR2=II
      RETURN
      ENDIF
      T=1.0D0/T
      ROW(RBASE+II)=T
      NIS=NDI+I
      CALLPCSSCL(NIN,JCOL(NIS),T,ROW(RBASE+1))
      DO7J=I+1,IBLKSK
      IPVTJ=IPVT(J)
      RBASEJ=(IPVTJ-1)*N
      T=-ROW(RBASEJ+II)
      IF(T.NE.0.0D0)THEN
      CALLPCPAXP(NIN,JCOL(NIS),T,ROW(RBASE+1),ROW(RBASEJ+1))
      ENDIF
7     CONTINUE
      IRSW(I1-1+I)=IRSW(I1-1+IPVTI)
      NIN=NIN-1
3     CONTINUE
      IPVTI=IPVT(IBLKSK)
      RBASE=(IPVTI-1)*N
      T=ROW(RBASE+I2)
      IF(T.EQ.0.0D0)THEN
      IERR=5
      IERR2=I2
      RETURN
      ENDIF
      T=1.0D0/T
      ROW(RBASE+I2)=T
      CALLPCSSCL(NIN,JCOL(NUI),T,ROW(RBASE+1))
      IRSW(I2)=I2
      RETURN
      END
