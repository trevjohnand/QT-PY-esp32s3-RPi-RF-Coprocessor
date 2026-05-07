param([string]$Port = "COM5")
idf.py -p $Port flash monitor
