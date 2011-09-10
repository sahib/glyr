-- Provider
create table if not exists providers (provider_name varchar(20) UNIQUE);

-- Artist
create table if not exists artists (artist_name varchar(128) UNIQUE);
create table if not exists albums (album_name varchar(128) UNIQUE);
create table if not exists titles (title_name varchar(128) UNIQUE);

-- Enum
create table if not exists image_types(image_type_name varchar(16) UNIQUE);

-- MetaData
create table if not exists metadata(
			          artist_id integer,
			          album_id integer,
			          title_id integer,
			          provider_id integer,
			          source_url varchar(512),
			          image_type_id integer,
			          data_type integer,
			          data_size integer,
			          data_checksum  varstring(16),
			          data blob
);

create index if not exists index_artist_id   on metadata(artist_id);
create index if not exists index_album_id    on metadata(album_id);
create index if not exists index_title_id    on metadata(title_id);
create index if not exists index_provider_id on metadata(provider_id);



-------------------------------------------------


insert into artists values ("Equilibrium");
insert into albums values ("Sagas");
insert into titles values ("Wurzelbert");
insert into provider values ("lastfm");

insert or ignore into image_types values("jpeg");
insert or ignore into image_types values("jpg");
insert or ignore into image_types values("png");
insert or ignore into image_types values("gif");
insert or ignore into image_types values("tiff");


