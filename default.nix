{
  cmake,
  lib,
  stdenv,
  kcli,
  ktest,
  ktl,
  doCheck ? false,
}:
stdenv.mkDerivation {
  name = "c-start";
  src = lib.cleanSource ./.;
  inherit doCheck;

  nativeBuildInputs = [ cmake ];

  buildInputs = [
    (kcli.override { inherit stdenv; })
    (ktest.override { inherit stdenv; })
    (ktl.override { inherit stdenv; })
  ];

  configurePhase = ''
    cmake -B build
  '';

  buildPhase = ''
    cmake --build build
  '';

  installPhase = ''
    if [[ "$CC" == *"mingw32"* ]]; then
      # Workaround broken pkgCross cmake install
      mkdir -p "$out/bin"
      cp build/app/*.exe "$out/bin"
      mkdir -p "$out/lib"
      cp build/src/*.a "$out/lib"
    else
      cmake --install build --prefix $out
    fi

    mkdir -p "$out/include"
    cp -r include "$out"
  '';

  checkPhase = ''
    (
      cd build/test
      ctest --output-on-failure
    )
  '';
}
