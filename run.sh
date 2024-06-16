mkdir -p run
cp scheduler run/
cd scripts
python3 create_graph.py
cd ../run
./scheduler
cp main_*.png ../website/static/
cd ../scripts
python3 totime.py
cd ..