      SUBROUTINEPCCG(N,IJA,A,RHS,X,ISP,RSP,PRECON,PCSTOP,
     *PCMV)
      IMPLICITDOUBLEPRECISION(A-H,O-Z)
      INTEGERN,IJA(*),ISP(*)
      DOUBLEPRECISIONA(*),RHS(*),X(*),RSP(*)
      EXTERNALPRECON,PCSTOP,PCMV
      INTEGERIERR,ITMAX,ITEST,KMAX,IFL,IPRE,PBLK,LUDONE,IBLK,
     *MNEED,ITS,RSDONE,NOVERF,RED1,RED2,IERR2,IFRMAT
      COMMON/PCCOM1/IERR,ITMAX,ITEST,KMAX,IFL,IPRE,PBLK,
     *LUDONE,IBLK,MNEED,ITS,RSDONE,NOVERF,RED1,RED2,IERR2,
     *IFRMAT
      SAVE/PCCOM1/
      INTEGERISTOP,IRESD,INORM,ISAME,ISTATP,IRSDS,IOUT,JOUT,ISTATS,
     *ISTAT1,ISTAT2
      DOUBLEPRECISIONEPS1,EPS2
      COMMON/PCSTIC/ISTOP,IRESD,INORM,ISAME,ISTATP,IRSDS,IOUT,JOUT,
     *ISTATS,ISTAT1,ISTAT2
      COMMON/PCSTRC/EPS1,EPS2
      SAVE/PCSTIC/,/PCSTRC/
      INTEGERNSP,RN,RP,P,ATP,NADR,NSP1
      NADR=ISP(2)
      NSP=ISP(1)
      RN=NADR+NSP-N
      IF(IPRE.NE.3)THEN
      RP=RN-N
      ATP=RP
      ELSE
      RP=RN
      ATP=RP-N
      ENDIF
      P=ATP-N
      NSP1=P-NADR
      ISP(3)=NSP1
      ISP(1)=NSP1
      IF(NSP1.LT.0)THEN
      RETURN
      ENDIF
      IF(IERR.NE.0)THEN
      RETURN
      ENDIF
      IRESD=0
      CALLPCCGX(N,IJA,A,RHS,X,RSP(RN),RSP(RP),RSP(P),RSP(ATP),ISP,
     *RSP,PRECON,PCSTOP,PCMV)
      RETURN
      END
