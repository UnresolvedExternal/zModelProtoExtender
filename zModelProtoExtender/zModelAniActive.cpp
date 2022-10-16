// Supported with union (c) 2020 Union team
// Union SOURCE file

namespace GOTHIC_ENGINE {
  zVEC3 zCModelAniActive::CalcTranslation() {
    if( !protoAni->aniFlags.flagVobPos || protoAni->rootNodeIndex == Invalid )
      return 0;

    const int currFrame = static_cast<int>( actFrame );

    if( currFrame < 0 || currFrame >= protoAni->numFrames )
      return 0;

    const int nextFrame = currFrame + 1;
    const int prevFrame = currFrame - 1;
    const float lerp = actFrame - static_cast<float>( currFrame );

    zVEC3 from = protoAni->GetTrans( currFrame, protoAni->rootNodeIndex );
    zVEC3 to = from;

    if( nextFrame < protoAni->numFrames )
      to = protoAni->GetTrans( nextFrame, protoAni->rootNodeIndex );
    else if( protoAni->numFrames >= 2 )
      to = protoAni->GetTrans( currFrame, protoAni->rootNodeIndex ) - protoAni->GetTrans( prevFrame, protoAni->rootNodeIndex ) + from;

    return from + ( to - from ) * lerp;
  }

  void zCModelAniActive::SetProgressPercent_Union( float progress ) {
    zCModelAniActive::SetProgressPercent( progress );
    lastPos = thisPos = CalcTranslation();

    if( advanceDir != 1 && advanceDir != -1 )
      return;

    while( actAniEvent >= 0 && actAniEvent < protoAni->numAniEvents ) {
      if( advanceDir == 1 && actFrame < protoAni->aniEvents[actAniEvent].frameNr )
        break;

      if( advanceDir == -1 && actFrame > protoAni->aniEvents[actAniEvent].frameNr )
        break;

      actAniEvent += advanceDir;
    }
  }
}