// Supported with union (c) 2020 Union team
// Union SOURCE file

namespace GOTHIC_ENGINE {
  void __fastcall Ivk_zTMdl_AniSample_Unpack(zTMdl_AniSample*, zCQuat&, zVEC3&)
  {
#if ENGINE == Engine_G1
    XCALL(0x0056A620)
#elif ENGINE == Engine_G1A
    XCALL(0x005834F0)
#elif ENGINE == Engine_G2
    XCALL(0x0057F970)
#elif ENGINE == Engine_G2A
    XCALL(0x00584ED0)
#endif
  }

  void __fastcall zTMdl_AniSample::Unpack_Union( zCModelAniActive* activeAni, int& dontUseAniSample, zCQuat& rotation, zVEC3& translation ) {
    Ivk_zTMdl_AniSample_Unpack( this, rotation, translation );

    zCModelAni* const protoAni = activeAni->protoAni;

    if( !dontUseAniSample || protoAni->numFrames < 2 )
      return;

    zTMdl_AniSample* badSample = protoAni->aniSampleMatrix;
    zTMdl_AniSample* currentSample = protoAni->aniSampleMatrix + protoAni->numNodes * ( protoAni->numFrames - 1 );

    if( activeAni->advanceDir == -1 )
      std::swap( badSample, currentSample );

    if( badSample != this )
      return;

    const zVEC3 newTrans = protoAni->GetAniTranslation() * static_cast<float>( activeAni->advanceDir ) / static_cast<float>( protoAni->numFrames - 1 );

    zCQuat dummyRotation;
    zVEC3 currentTranslation;
    Ivk_zTMdl_AniSample_Unpack( currentSample, dummyRotation, currentTranslation );

    translation = currentTranslation + newTrans;
    dontUseAniSample = false;
  }
}
