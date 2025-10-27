# image-db

This is a fun side project that I want to write to revive my open source programming days and also to learn more complex C++ and asynchronous programming.

I was reading designing data intensive systems book and wasn't to solid my understanding through this complex project.

# Thought process

1) Build a fast, queryable image database store, Think SQL for images

2) Index, tag images and cluster it intenally, allowing me to fetch images by tags, quality or random chance.

3) Implement replication and failure prevention mode by using a Write Ahead Log

# Usage

1) Initialize a new db
./imgdb -cmd init -root "/Users/kaushrk/projects/imgdb"

2) Add image to your database
./imgdb -cmd import -root "/Users/kaushrk/projects/imgdb" -img "/Users/kaushrk/projects/img.jpg"
