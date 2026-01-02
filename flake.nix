{
  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixos-unstable";
    flake-utils.url = "github:numtide/flake-utils";

    kcli = {
      url = "github:krscott/kcli";
      inputs.nixpkgs.follows = "nixpkgs";
    };

    ktl = {
      url = "github:krscott/ktl";
      inputs.nixpkgs.follows = "nixpkgs";
    };

    ktest = {
      url = "github:krscott/ktest";
      inputs.nixpkgs.follows = "nixpkgs";
    };
  };

  outputs =
    {
      self,
      nixpkgs,
      flake-utils,
      kcli,
      ktl,
      ktest,
    }:
    let
      supportedSystems = [
        "x86_64-linux"
        "aarch64-linux"
        "x86_64-darwin"
        "aarch64-darwin"
      ];
    in
    flake-utils.lib.eachSystem supportedSystems (
      system:
      let
        pkgs = nixpkgs.legacyPackages.${system};

        # Final derivation including any overrides made to output package
        inherit (self.packages.${system}) ofx2csv ofx2csv-gcc;

        devPkgs =
          with pkgs;
          [
            shfmt
            cmake-format
            clang-tools # NOTE: clang-tools must come before clang
            clang
          ]
          ++ ofx2csv.buildInputs;

        mkApp = text: {
          type = "app";
          program = pkgs.lib.getExe (
            pkgs.writeShellApplication {
              name = "app";
              runtimeInputs = devPkgs;
              inherit text;
            }
          );
        };
      in
      {
        packages = {
          ofx2csv = pkgs.callPackage ./. {
            inherit (kcli.packages.${system}) kcli;
            inherit (ktest.packages.${system}) ktest;
            inherit (ktl.packages.${system}) ktl;
            stdenv = pkgs.clangStdenv;
          };

          ofx2csv-gcc = ofx2csv.override {
            inherit (pkgs) stdenv;
          };

          ofx2csv-win = ofx2csv.override {
            inherit (pkgs.pkgsCross.mingwW64) stdenv;
          };

          default = ofx2csv;

          ofx2csv-test = ofx2csv.override {
            doCheck = true;
          };
          ofx2csv-gcc-test = ofx2csv-gcc.override {
            doCheck = true;
          };
        };

        devShells = {
          default = pkgs.mkShell {
            inputsFrom = [ ofx2csv ];
            nativeBuildInputs = devPkgs;
            shellHook = ''
              source dev_shell.sh
            '';
          };
        };

        apps = {
          format = mkApp ''
            ./format.sh
          '';
        };

        formatter = pkgs.nixfmt-rfc-style;
      }
    );
}
