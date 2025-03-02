#!/bin/bash
set -e

BASE_PREFIX=$(conda info --base)
if [[ "$CONDA_PREFIX" == "$BASE_PREFIX" ]]; then
    echo "Please run this script inside a non-base conda environment."
    exit 1
fi


mkdir -p $CONDA_PREFIX/etc/conda/activate.d/
mkdir -p $CONDA_PREFIX/etc/conda/deactivate.d/
cat <<EOF > $CONDA_PREFIX/etc/conda/activate.d/vpn_goroot.sh
#!/bin/bash
export GOBIN=$CONDA_PREFIX/bin
EOF
chmod +x $CONDA_PREFIX/etc/conda/activate.d/vpn_goroot.sh

cat <<EOF > $CONDA_PREFIX/etc/conda/deactivate.d/vpn_goroot.sh
#!/bin/bash
unset GOBIN
EOF
chmod +x $CONDA_PREFIX/etc/conda/deactivate.d/vpn_goroot.sh


export GOBIN=$CONDA_PREFIX/bin
go install golang.org/x/mobile/cmd/gomobile@latest
gomobile init
