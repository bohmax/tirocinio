# tirocinio
Simulatore per trasmissioni multimediali Real-Time da drone a terra
## Dipendenze
Python 3.7

Gstreamer 1.16
'''
sudo apt-get install libgstreamer1.0-0 libgstreamer1.0-0 gstreamer1.0-plugins-base gstreamer1.0-plugins-good gstreamer1.0-plugins-bad gstreamer1.0-plugins-ugly gstreamer1.0-libav gstreamer1.0-doc gstreamer1.0-tools gstreamer1.0-x gstreamer1.0-alsa gstreamer1.0-gl gstreamer1.0-gtk3 gstreamer1.0-qt5 gstreamer1.0-pulseaudio
'''
libpcap 1.8.1
'''
sudo apt-get install libpcap-dev
'''
ffmpeg 4.4.2
'''
sudo apt-get update -qq && sudo apt-get -y install autoconf automake yasm nasm cmake git-core libass-dev libfreetype6-dev libgnutls28-dev libsdl2-dev libtool libva-dev libvdpau-dev libvorbis-dev libxcb1-dev libxcb-shm0-dev libxcb-xfixes0-dev pkg-config texinfo wget libx264-dev libvpx-dev zlib1g-dev
    wget https://ffmpeg.org/releases/ffmpeg-4.2.2.tar.bz2
    tar -xf ffmpeg-4.2.2.tar.bz2
    rm ffmpeg-4.2.2.tar.bz2
    cd ffmpeg-4.2.2
    ./configure --enable-gpl --enable-gnutls --enable-libass --enable-libfreetype --enable-libvorbis --enable-libvpx --enable-libx264 --enable-shared --enable-nonfree --enable-pthreads
    make
    sudo make install
    sudo /sbin/ldconfig
    cd ..
    rm -rf ffmpeg-4.2.2
'''
PyQt5
'''
sudo apt-get install python3-pyqt5
'''
I seguenti pacchetti si installano utilizzando pip
'''
sudo pip3 install scapy
sudo pip3 install pyqtgraph
sudo pip3 install scipy
sudo pip3 install opencv-python
'''
##Esecuzione
Prima di eseguire il simulatore estrarre il file Simulatore.zip eseguendo i seguenti comandi e ci si rechi nella cartella appena creata coi comandi:
'''
unzip Simulatore.zip
cd Simulatore
chmod +x execute.sh
'''
Si entri nella cartella Config e si modifichino i dati dei file di configurazione, successivamente nell cartella simulatore si esegua su due shell separate
'''
./execute.sh 1 Config/sender.conf Config/receiver.conf
./execute.sh 2 Config/sender.conf Config/receiver.conf
'''