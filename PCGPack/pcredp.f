      SUBROUTINEPCREDP(N,IJA,A,RHS,X,ISP,RSP,METHOD,PRECON,
     *PCSTOP,PCMV)
      IMPLICITDOUBLEPRECISION(A-H,O-Z)
      INTEGERN,IJA(*),ISP(*)
      DOUBLEPRECISIONA(*),RHS(*),X(*),RSP(*)
      EXTERNALMETHOD,PRECON,PCSTOP,PCMV
      INTEGERIERR,ITMAX,ITEST,KMAX,IFL,IPRE,PBLK,LUDONE,IBLK,
     *MNEED,ITS,RSDONE,NOVERF,RED1,RED2,IERR2,IFRMAT
      COMMON/PCCOM1/IERR,ITMAX,ITEST,KMAX,IFL,IPRE,PBLK,
     *LUDONE,IBLK,MNEED,ITS,RSDONE,NOVERF,RED1,RED2,IERR2,
     *IFRMAT
      SAVE/PCCOM1/
      INTEGERIPRE1,IPRE2,RATIO
      COMMON/PCCOM3/IPRE1,IPRE2,RATIO
      SAVE/PCCOM3/
      INTEGERISRT,IJASGN,ISX0,IFORM
      COMMON/PCVERI/ISRT,IJASGN,ISX0,IFORM
      SAVE/PCVERI/
      INTEGERNADR,INADR
      INTEGERIBLKN,IC,C,IPVT,RSRHS,IJRS,RS,Y,RSNADR,NSP1,
     *NBLACK,IMN,ISCR,D2I,NZRS,I,NRED
      INTEGERISAVE(31)
      INTEGERIRS1,IRS2
      INTEGERSISRT,SNVRFY,SIFORM
      EQUIVALENCE(IBLKN,ISAVE(8)),(IC,ISAVE(9)),(C,ISAVE(10)),(D2I,
     *ISAVE(11)),(IPVT,ISAVE(12)),(RSRHS,ISAVE(14)),(NZRS,ISAVE(15)),(
     *IJRS,ISAVE(16)),(RS,ISAVE(17)),(Y,ISAVE(18)),(RSNADR,ISAVE(19)),(
     *NSP1,ISAVE(20)),(NBLACK,ISAVE(27)),(IMN,ISAVE(28)),(ISCR,ISAVE(29
     *))
      IRS1=8
      IRS2=31
      NADR=ISP(2)
      INADR=(NADR-1)*RATIO+1
      IF(IJASGN.EQ.0)THEN
      CALLPCIABS(N,IJA)
      ENDIF
      IF(RSDONE.EQ.1)THEN
      DO1I=IRS1,IRS2
      ISAVE(I)=ISP(I)
1     CONTINUE
      NRED=N-NBLACK
      CALLPCPRMP(NRED,ISP(C+NBLACK),RHS,RSP(Y))
      CALLPCPRMP(NBLACK,ISP(C),RHS,RSP(RSRHS))
      CALLPCVMUL(NRED,RSP(Y),RSP(Y),RSP(D2I))
      CALLPCRC1P(NBLACK,N,ISP(C),ISP(IC),IJA,A,RSP(Y),RSP(
     *RSRHS))
      ELSE
      IC=INADR
      IF(RSDONE.EQ.2)THEN
      DO3I=IRS1,IRS2
      ISAVE(I)=ISP(I)
3     CONTINUE
      ELSE
      LUDONE=0
      ENDIF
      CALLPCRSFP(N,IJA,A,RHS,ISP,RSP,IC,C,NBLACK,D2I,NZRS,
     *IJRS,RS,RSRHS,Y,IMN,ISCR,RSNADR,NSP1)
      IF(IERR.NE.0)THEN
      RETURN
      ENDIF
      IBLKN=N
      DO5I=IRS1,IRS2
      ISP(I)=ISAVE(I)
5     CONTINUE
      ENDIF
      CALLPCPRMP(NBLACK,ISP(C),X,RSP(Y))
      IF(NBLACK.NE.0)THEN
      SISRT=ISRT
      SNVRFY=NOVERF
      SIFORM=IFORM
      ISRT=0
      NOVERF=2
      IFORM=0
      ISP(1)=NSP1
      ISP(2)=RSNADR
      CALLPCDRVX(NBLACK,ISP(IJRS),RSP(RS),RSP(RSRHS),RSP(Y),ISP,
     *RSP,METHOD,PRECON,PCSTOP,PCMV)
      ISRT=SISRT
      NOVERF=SNVRFY
      IFORM=SIFORM
      IF(IERR.LT.2.OR.IERR.GT.4)THEN
      MNEED=MAX(MNEED,ISCR)+IMN
      ENDIF
      CALLPCIPRP(NBLACK,ISP(C),RSP(Y),X)
      ELSE
      MNEED=IMN+ISCR
      ENDIF
      NRED=N-NBLACK
      CALLPCRDPP(NRED,ISP(C+NBLACK),IJA,A,RHS,RSP(D2I),X)
      RETURN
      END