FROM ubuntu:18.04

WORKDIR /opt/s2n

ADD . codebuild

ENV S2N_LIBCRYPTO=openssl-1.1.1 \
    BUILD_S2N=true \
    GCC_VERSION=9 \
    TESTS=integration \
    ZSH_THEME=cypher

RUN set -eux; \
  # we need to install sudo because the codebuild scripts use it
  apt-get update; \
  apt-get -y install curl sudo zsh unzip gnupg2 software-properties-common python-pip rubygems wget; \
  gem install bundler; \
  bash codebuild/bin/s2n_setup_env.sh; \
  codebuild/bin/install_ubuntu_dependencies.sh; \
  codebuild/bin/s2n_install_test_dependencies.sh; \
  # setup user account
  useradd -m s2n-dev; \
  echo "s2n-dev ALL=(ALL) NOPASSWD:ALL" > /etc/sudoers.d/s2n-dev; \
  chmod 0440 /etc/sudoers.d/s2n-dev; \
  mkdir -p /home/s2n-dev/s2n; \
  curl -L https://github.com/ohmyzsh/ohmyzsh/archive/master.zip -o /tmp/oh-my-zsh.zip; \
  unzip /tmp/oh-my-zsh.zip -d /tmp; \
  mkdir -p /home/s2n-dev/.zsh/plugins; \
  mkdir -p /home/s2n-dev/.zsh/themes; \
  cp /tmp/ohmyzsh-master/oh-my-zsh.sh /home/s2n-dev/.zsh/init.sh; \
  cp -R /tmp/ohmyzsh-master/lib /home/s2n-dev/.zsh; \
  cp /tmp/ohmyzsh-master/themes/$ZSH_THEME.zsh-theme /home/s2n-dev/.zsh/themes/theme.zsh-theme; \
  rm -rf /tmp/oh-my-zsh.zip /tmp/ohmyzsh-master; \
  echo '\n# ZSH setup\n\
  export DISABLE_UPDATE_PROMPT=true\n\
  export DISABLE_AUTO_UPDATE=true\n\
  export ZSH="/home/s2n-dev/.zsh"\n\
  export ZSH_THEME="theme"\n\
  plugins=()\n\
  source $ZSH/init.sh\n\
  \n\
  \n# s2n setup\n\
  export S2N_LIBCRYPTO='"$S2N_LIBCRYPTO"'\n\
  export BUILD_S2N=true\n\
  export GCC_VERSION='"$GCC_VERSION"'\n\
  export TESTS=integration\n\
  export TEST_DEPS_DIR=/opt/s2n/test-deps\n\
  cd /home/s2n-dev/s2n && source /opt/s2n/codebuild/bin/s2n_setup_env.sh\n\
  ' > /home/s2n-dev/.zshrc; \
  chown -R s2n-dev:s2n-dev /home/s2n-dev; \
  rm -rf /var/lib/apt/lists/*; \
  apt-get clean; \
  echo done

USER s2n-dev
WORKDIR /home/s2n-dev/s2n
CMD ["/bin/zsh","-l"]
