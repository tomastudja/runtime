# Builds and copies library artifacts into target dotnet sdk image
ARG BUILD_BASE_IMAGE=mcr.microsoft.com/dotnet-buildtools/prereqs:centos-7-f39df28-20191023143754
ARG SDK_BASE_IMAGE=mcr.microsoft.com/dotnet/core/sdk:3.0.100-buster

FROM $BUILD_BASE_IMAGE as corefxbuild

WORKDIR /repo
COPY . .

ARG CONFIGURATION=Release
RUN ./src/coreclr/build.sh -release -skiptests -clang9 && \
    ./libraries.sh -c $CONFIGURATION -runtimeconfiguration release

FROM $SDK_BASE_IMAGE as target

ARG TESTHOST_LOCATION=/repo/artifacts/bin/testhost
ARG TFM=net5.0
ARG OS=Linux
ARG ARCH=x64
ARG CONFIGURATION=Release

ARG COREFX_SHARED_FRAMEWORK_NAME=Microsoft.NETCore.App
ARG ASPNETCORE_SHARED_NAME=Microsoft.AspNetCore.App
ARG SOURCE_COREFX_VERSION=5.0.0
ARG TARGET_SHARED_FRAMEWORK=/usr/share/dotnet/shared
ARG TARGET_COREFX_VERSION=3.0.0

COPY --from=corefxbuild \
    $TESTHOST_LOCATION/$TFM-$OS-$CONFIGURATION-$ARCH/shared/$COREFX_SHARED_FRAMEWORK_NAME/$SOURCE_COREFX_VERSION/* \
    $TARGET_SHARED_FRAMEWORK/$COREFX_SHARED_FRAMEWORK_NAME/$TARGET_COREFX_VERSION/
COPY --from=corefxbuild \
    $TESTHOST_LOCATION/$TFM-$OS-$CONFIGURATION-$ARCH/shared/$COREFX_SHARED_FRAMEWORK_NAME/$SOURCE_COREFX_VERSION/* \
    $TARGET_SHARED_FRAMEWORK/$COREFX_SHARED_FRAMEWORK_NAME/$SOURCE_COREFX_VERSION/
COPY --from=corefxbuild \
    $TESTHOST_LOCATION/$TFM-$OS-$CONFIGURATION-$ARCH/shared/$ASPNETCORE_SHARED_NAME/$SOURCE_COREFX_VERSION/* \
    $TARGET_SHARED_FRAMEWORK/$ASPNETCORE_SHARED_NAME/$TARGET_COREFX_VERSION/