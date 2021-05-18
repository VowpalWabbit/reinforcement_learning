set(RL_NET_SOURCES
  Native/ErrorCallback.cs
  Native/GCHandleLifetime.cs
  Native/Global.cs
  Native/NativeObject.cs
  Native/SenderAdapter.cs
  Native/StringExtensions.cs

  ActionFlags.cs
  ApiStatus.cs
  AsyncSender.cs
  Configuration.cs
  ContinuousActionResponse.cs
  DecisionResponse.cs
  FactoryContext.cs
  InternalsVisibleToTest.tt
  ISender.cs
  LiveModel.cs
  LiveModelThreadSafe.cs
  MultiSlotResponse.cs
  MultiSlotResponseDetailed.cs
  RankingResponse.cs
  RLException.cs
  RLLibLogUtils.cs
  SharedBuffer.cs
  SlotRanking.cs
  TraceLogEventArgs.cs
)

add_custom_target(rl.net
    COMMAND ${DOTNET_COMMAND} build ${CMAKE_CURRENT_SOURCE_DIR} -o $<TARGET_FILE_DIR:rl.net.native> -v m --nologo --no-dependencies /clp:NoSummary
    COMMENT Building rl.net
    DEPENDS rl.net.native
    SOURCES ${RL_NET_SOURCES})