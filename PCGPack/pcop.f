      SUBROUTINEPCOP(N,IJA,A,V,W1,ISP,RSP,PRECON,PCMV)
      IMPLICITDOUBLEPRECISION(A-H,O-Z)
      INTEGERIJA(*),ISP(*),N
      DOUBLEPRECISIONV(*),W1(*),A(*),RSP(*)
      EXTERNALPRECON,PCMV
      INTEGERIPRE1,IPRE2,RATIO
      COMMON/PCCOM3/IPRE1,IPRE2,RATIO
      SAVE/PCCOM3/
      IF(IPRE1.NE.-4)THEN
      CALLPRECON(IPRE1,N,IJA,A,V,ISP,RSP)
      ENDIF
      CALLPCMV(N,IJA,A,V,W1)
      IF(IPRE2.NE.-4)THEN
      CALLPRECON(IPRE2,N,IJA,A,W1,ISP,RSP)
      ENDIF
      RETURN
      END
