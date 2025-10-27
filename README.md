# image-db

This is a fun side project that I want to write to revive my open source programming days and also to learn more complex C++ and asynchronous programming.

I was reading designing data intensive systems book and wasn't to solid my understanding through this complex project.

# Thought process

1) Build a fast, queryable image database store, Think SQL for images

2) Index, tag images and cluster it intenally, allowing me to fetch images by tags, quality or random chance.

3) Implement replication and failure prevention mode by using a Write Ahead Log

# Usage

imgdb init --path /Users/rohit/data/imgdb
imgdb import --db /Users/rohit/data/imgdb --file ~/Pictures/cat.jpg
