mkdir -p run
cp scheduler run/
cd scripts
python3 create_graph.py
cd ../run
./scheduler
cd ../scripts
python3 totime.py
cd ..