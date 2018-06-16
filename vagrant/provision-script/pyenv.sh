#! /bin/bash

PYTHON="anaconda3-5.0.1"

if [ ! -e '/home/vagrant/.pyenv' ]; then
	git clone https://github.com/yyuu/pyenv.git ~/.pyenv
	echo 'export PYENV_ROOT="$HOME/.pyenv"' >> ~/.bash_profile
	echo 'export PATH="$PYENV_ROOT/bin:$PATH"' >> ~/.bash_profile
	echo 'eval "$(pyenv init -)"' >> ~/.bash_profile
	source ~/.bash_profile

	# anacondaのバージョン確認
	# pyenv install -l | grep anaconda
	# 最新版インストール
	pyenv install $PYTHON
	pyenv rehash
	# anacondaをpythonのデフォルトに設定
	pyenv global $PYTHON
	echo 'export PATH="$PYENV_ROOT/versions/$PYTHON/bin/:$PATH"' >> ~/.bashrc
	source ~/.bash_profile
	conda update conda
	pip install --upgrade pip

	pip install chainer, cupy, chainerrl
	pip install pybind11
	pip install pyprind

	conda update -y mkl
	conda install -y numpy
	conda install -y gensim
	conda update -y dask
fi
